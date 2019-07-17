#include "utilize_indices.h"
#include "../ops/op_index_scan.h"
#include "../../parser/grammar.h"
#include "../../util/arr.h"

/* Reverse an inequality symbol so that indices can support
 * inequalities with right-hand variables. */
int _reverseOp(int op) {
    switch(op) {
        case LT:
            return GT;
        case LE:
            return GE;
        case GT:
            return LT;
        case GE:
            return LE;
        default:
            return op;
    }
}

void _locateScanFilters(NodeByLabelScan *scanOp, OpBase ***filterOps) {
    /* We begin with a LabelScan, and want to find predicate filters that modify
    * the active entity. */
    OpBase *current = scanOp->op.parent;
    // TODO: Not sure if this while is necessary.
    while(current->type == OPType_FILTER) {
        Filter *filterOp = (Filter*)current;
        FT_FilterNode *filterTree = filterOp->filterTree;

        /* filterTree will either be a predicate or a tree with an OR root.
         * We'll store ops on const predicate filters, and can otherwise safely ignore them -
         * no filter tree in this sequence can invalidate another. */
        if (IsNodePredicate(filterTree)) {
            *filterOps = array_append(*filterOps, current);
        }

        // Advance to the next operation.
        current = current->parent;
    }
}

// Populate scanOps array with execution plan scan operations.
void _locateScanOp(OpBase *root, NodeByLabelScan ***scanOps) {

    // Is this a scan operation?
    if (root->type == OPType_NODE_BY_LABEL_SCAN) {
        *scanOps = array_append(*scanOps, (NodeByLabelScan*)root);
    }

    // Continue scanning.
    for (int i = 0; i < root->childCount; i++) {
        _locateScanOp(root->children[i], scanOps);
    }
}

void utilizeIndices(ExecutionPlan *plan, AST *ast) {
    Index *idx = NULL;
    GraphContext *gc = GraphContext_GetFromTLS();

    // Return immediately if the graph has no indices
    if (!GraphContext_HasIndices(gc)) return;

    // Collect all label scans
    NodeByLabelScan **scanOps = array_new(NodeByLabelScan*, 0);
    _locateScanOp(plan->root, &scanOps);

    // Collect all filters on scanned entities
    NodeByLabelScan *scanOp;
    OpBase **filterOps = array_new(OpBase*, 0);
    FT_FilterNode *ft;
    char *label;

    // Variables to be used when comparing filters against available indices
    char *filterProp = NULL;
    int lhsType, rhsType;
    int scanOpCount = array_len(scanOps);
    for(int i = 0; i < scanOpCount; i++) {
        scanOp = scanOps[i];

        /* Get the label string for the scan target.
        * The label will be used to retrieve the index. */
        label = scanOp->node->label;
        array_clear(filterOps);
        _locateScanFilters(scanOp, &filterOps);

        // No filters.
        if(array_len(filterOps) == 0) continue;

        /* At this point we have all the filter ops (and thus, filter trees) associated
        * with the scanned entity. If there are valid indices on any filter and no
        * equal or higher precedence OR filters, we can switch to an index scan.
        *
        * We'll currently use the first matching index, but apply all the filters on
        * that property. A later optimization would be to find the index with the
        * most filters, or use some heuristic for trying to select the minimal range. */

        int filterOpsCount = array_len(filterOps);
        for (int i = 0; i < filterOpsCount; i ++) {
            OpBase *opFilter = filterOps[i];
            ft = ((Filter *)opFilter)->filterTree;
            /* We'll only employ indices when we have filters of the form:
            * node.property [rel] constant or
            * constant [rel] node.property
            * If we are not comparing against a constant, then we cannot pre-define useful bounds
            * for the index iterator, which diminishes their utility. */
            lhsType = AR_EXP_GetOperandType(ft->pred.lhs);
            rhsType = AR_EXP_GetOperandType(ft->pred.rhs);
            if (lhsType == AR_EXP_VARIADIC && rhsType == AR_EXP_CONSTANT) {
                filterProp = ft->pred.lhs->operand.variadic.entity_prop;
            } else if (lhsType == AR_EXP_CONSTANT && rhsType == AR_EXP_VARIADIC) {
                filterProp = ft->pred.rhs->operand.variadic.entity_prop;
            } else {
                continue;
            }

            // Property isn't indexed.
            if(idx && !Index_ContainsField(idx, filterProp)) continue;

            // Try to retrieve an index if one has not been selected yet
            if (!idx) {
                idx = GraphContext_GetIndex(gc, label, filterProp, IDX_EXACT_MATCH);
                if (!idx) continue;
            }

            ExecutionPlan_RemoveOp(plan, opFilter);
            OpBase_Free(opFilter);
        }

        if(idx != NULL) {
           OpBase *indexOp = NewIndexScanOp(scanOp->g, scanOp->node, idx->idx, NULL, ast);
            ExecutionPlan_ReplaceOp(plan, (OpBase*)scanOp, indexOp);
        }
    }

    // Cleanup
    array_free(filterOps);
    array_free(scanOps);
}

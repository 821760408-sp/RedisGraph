/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_fulltext_query.h"
#include "../value.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../index/fulltext_index.h"

//------------------------------------------------------------------------------
// fulltext createNodeIndex
//------------------------------------------------------------------------------

// CALL db.idx.fulltext.queryNodes(label, query)

typedef struct {
    Node n;
    Graph *g;
    SIValue *output;
    FullTextIndex *idx;
    RSResultsIterator *iter;
} QueryNodeContext;

ProcedureResult Proc_FulltextQueryNodeInvoke(ProcedureCtx *ctx, char **args) {
    if(array_len(args) < 2) return PROCEDURE_ERR;

    ctx->privateData = NULL;
    GraphContext *gc = GraphContext_GetFromTLS();

    // See if there's a full-text index for given label.
    char* err = NULL;
    const char *label = args[0];
    const char *query = args[1];

    // Get full-text index from schema.
    Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
    if(s == NULL) return PROCEDURE_OK;
    FullTextIndex *idx = Schema_GetFullTextIndex(s);
    if(!idx) return PROCEDURE_OK;

    QueryNodeContext *pdata = rm_malloc(sizeof(QueryNodeContext));
    pdata->idx = idx;
    pdata->g = gc->g;
    pdata->output = array_new(SIValue, 2);
    pdata->output = array_append(pdata->output, SI_ConstStringVal("node"));
    pdata->output = array_append(pdata->output, SI_Node(&pdata->n));
    // pdata->output = array_append(pdata->output, SI_ConstStringVal("score"));
    // pdata->output = array_append(pdata->output, SI_DoubleVal(0.0));

    // Execute query
    pdata->iter = FullTextIndex_Query(pdata->idx, query, &err);
    // TODO: report error!
    if(err) pdata->iter = NULL;

    ctx->privateData = pdata;
    return PROCEDURE_OK;
}

SIValue* Proc_FulltextQueryNodeStep(ProcedureCtx *ctx) {
    assert(ctx->privateData);

    QueryNodeContext *pdata = (QueryNodeContext*)ctx->privateData;
    if(!pdata || !pdata->iter) return NULL;

    /* Try to get a result out of the iterator.
     * NULL is returned if iterator id depleted. */
    size_t len = 0;
    NodeID *id = (NodeID*)RediSearch_ResultsIteratorNext(pdata->iter, pdata->idx->idx, &len);

    // Depleted.
    if(!id) return NULL;

    // Get Node.
    Node *n = &pdata->n;
    Graph_GetNode(pdata->g, *id, n);

    pdata->output[1] = SI_Node(n);
    return pdata->output;
}

ProcedureResult Proc_FulltextQueryNodeFree(ProcedureCtx *ctx) {
    // Clean up.
    if(!ctx->privateData) return PROCEDURE_OK;

    QueryNodeContext *pdata = ctx->privateData;
    array_free(pdata->output);
    if(pdata->iter) RediSearch_ResultsIteratorFree(pdata->iter);
    rm_free(pdata);

    return PROCEDURE_OK;
}

ProcedureCtx* Proc_FulltextQueryNodeGen() {
    void *privateData = NULL;
    ProcedureOutput **output = array_new(ProcedureOutput*, 1);
    ProcedureOutput *out_node = rm_malloc(sizeof(ProcedureOutput));
    out_node->name = "node";
    out_node->type = T_NODE;

    output = array_append(output, out_node);
    ProcedureCtx *ctx = ProcCtxNew("db.idx.fulltext.queryNodes",
                                    2,
                                    output,
                                    Proc_FulltextQueryNodeStep,
                                    Proc_FulltextQueryNodeInvoke,
                                    Proc_FulltextQueryNodeFree,
                                    privateData);
    return ctx;
}

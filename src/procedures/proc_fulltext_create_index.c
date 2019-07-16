/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_fulltext_create_index.h"
#include "../value.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../index/fulltext_index.h"

//------------------------------------------------------------------------------
// fulltext createNodeIndex
//------------------------------------------------------------------------------

// CALL db.idx.fulltext.createNodeIndex(index_name, label, attributes)
// CALL db.idx.fulltext.createNodeIndex('books', ['Book'], ['title', 'authors'])
ProcedureResult Proc_FulltextCreateNodeIdxInvoke(ProcedureCtx *ctx, char **args) {
    if(array_len(args) < 2) return PROCEDURE_ERR;

    // Create full-text index.
    char *label = args[0];
    uint fields_count = array_len(args) - 1;
    char **fields = args+1; // Skip index name.

    GraphContext *gc = GraphContext_GetFromTLS();
    Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
    FullTextIndex *idx = Schema_GetFullTextIndex(s);

    // Schema doesn't contains a fulltext index create one.
    if(idx == NULL) {
        idx = FullTextIndex_New(label);
        Schema_SetFullTextIndex(s, idx);
    }

    // Introduce fields to index.
    for(int i = 0; i < fields_count; i++) {
        char *field = fields[i];
        FullTextIndex_AddField(idx, field);
    }

    // Build index.
    FullTextIndex_Construct(idx);

    return PROCEDURE_OK;
}

SIValue* Proc_FulltextCreateNodeIdxStep(ProcedureCtx *ctx) {
    return NULL;
}

ProcedureResult Proc_FulltextCreateNodeIdxFree(ProcedureCtx *ctx) {
    // Clean up.
    return PROCEDURE_OK;
}

ProcedureCtx* Proc_FulltextCreateNodeIdxGen() {
    void *privateData = NULL;
    ProcedureOutput **output = array_new(ProcedureOutput*, 0);
    ProcedureCtx *ctx = ProcCtxNew("db.idx.fulltext.createNodeIndex",
                                    PROCEDURE_VARIABLE_ARG_COUNT,
                                    output,
                                    Proc_FulltextCreateNodeIdxStep,
                                    Proc_FulltextCreateNodeIdxInvoke,
                                    Proc_FulltextCreateNodeIdxFree,
                                    privateData);

    return ctx;
}

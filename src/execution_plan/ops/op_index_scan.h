/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_INDEX_SCAN_H
#define __OP_INDEX_SCAN_H

#include "op.h"
#include "../../graph/graph.h"
#include "../../index/index.h"
#include "../../graph/entities/node.h"


typedef struct {
    OpBase op;
    Graph *g;
    QGNode *n;
    uint recLength;  // Number of entries in a record.
    uint nodeRecIdx;
    IndexIter *iter;
} IndexScan;

/* Creates a new IndexScan operation */
OpBase *NewIndexScanOp(Graph *g, QGNode *n, uint node_idx, IndexIter *iter);

/* IndexScan next operation
 * called each time a new node is required */
Record IndexScanConsume(OpBase *opBase);

/* Restart iterator */
OpResult IndexScanReset(OpBase *ctx);

/* Frees IndexScan */
void IndexScanFree(OpBase *ctx);

#endif
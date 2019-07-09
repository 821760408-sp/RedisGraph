/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "qg_edge.h"
#include "qg_node.h"
#include "../graph.h"
#include "../../util/arr.h"
#include <assert.h>

QGEdge* QGEdge_New(QGNode *src, QGNode *dest, const char *relationship, const char *alias, uint id) {
    QGEdge *e = rm_malloc(sizeof(QGEdge));
    e->id = id;
    e->alias = alias;
    e->reltypes = array_new(const char*, 1);
    e->reltypeIDs = array_new(uint, 1);
    e->src = NULL;
    e->dest = NULL;
    e->minHops = 1;
    e->maxHops = 1;

    return e;
}

QGEdge* QGEdge_Clone(const QGEdge *orig) {
    QGEdge *e = rm_malloc(sizeof(QGEdge));
    e->alias = orig->alias;
    e->reltypes = orig->reltypes; // TODO possibly memcpy
    e->reltypeIDs = orig->reltypeIDs; // TODO possibly memcpy
    e->minHops = orig->minHops;
    e->maxHops = orig->maxHops;
    e->id = orig->id;
    e->src = NULL;
    e->dest = NULL;

    return e;
}

bool QGEdge_VariableLength(const QGEdge *e) {
	assert(e);
	return (e->minHops != e->maxHops);
}

void QGEdge_Reverse(QGEdge *e) {
	QGNode *src = e->src;
	QGNode *dest = e->dest;

	QGNode_RemoveOutgoingEdge(src, e);
	QGNode_RemoveIncomingEdge(dest, e);

	// Reconnect nodes with the source and destination reversed.
	e->src = dest;
	e->dest = src;

	QGNode_ConnectNode(e->src, e->dest, e);
}

int QGEdge_ToString(const QGEdge *e, char *buff, int buff_len) {
    assert(e && buff);

    int offset = 0;
    offset += snprintf(buff + offset, buff_len - offset, "[");

    if(e->alias)
        offset += snprintf(buff + offset, buff_len - offset, "%s", e->alias);
    uint reltype_count = array_len(e->reltypes);
    for (uint i = 0; i < reltype_count; i ++) {
        offset += snprintf(buff + offset, buff_len - offset, ":%s", e->reltypes[i]); // TODO how should this print?
    }
    if(e->minHops !=1 || e->maxHops !=1) {
        if(e->maxHops == EDGE_LENGTH_INF)
            offset += snprintf(buff + offset, buff_len - offset, "*%u..INF", e->minHops);
        else
            offset += snprintf(buff + offset, buff_len - offset, "*%u..%u", e->minHops, e->maxHops);
    }

    offset += snprintf(buff + offset, buff_len - offset, "]");
    return offset;
}

void QGEdge_Free(QGEdge* e) {
    if(!e) return;

    // TODO
    // array_free(e->reltypes);
    // array_free(e->reltypeIDs);

    rm_free(e);
}

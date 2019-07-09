/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include <stdlib.h>
#include <stdint.h>

/* Forward declaration of edge */
typedef struct QGEdge QGEdge;

typedef struct {
    uint id;
    int labelID;
    const char *label; // TODO move
    const char *alias;
    QGEdge** outgoing_edges;   /* Array of incoming edges (ME)<-(SRC) */
    QGEdge** incoming_edges;   /* Array of outgoing edges (ME)->(DEST) */
} QGNode;

/* Creates a new node. */
QGNode* QGNode_New(const char *label, const char *alias, uint id);

/* Returns number of edges pointing into node. */
int QGNode_IncomeDegree(const QGNode *n);

/* Returns number of edges pointing out of node. */
int QGNode_OutgoingDegree(const QGNode *n);

/* Returns to total number of edges (Incoming&Outgoing). */
int QGNode_EdgeCount(const QGNode *n);

/* Connects source node to destination node by edge. */
void QGNode_ConnectNode(QGNode* src, QGNode* dest, QGEdge* e);

/* Removes given Incoming edge from node. */
void QGNode_RemoveIncomingEdge(QGNode *n, QGEdge *e);

/* Removes given Outgoing edge from node. */
void QGNode_RemoveOutgoingEdge(QGNode *n, QGEdge *e);

/* Clones given node. */
QGNode* QGNode_Clone(const QGNode *n);

/* Gets a string representation of given node. */
int QGNode_ToString(const QGNode *n, char *buff, int buff_len); // Ops printing

/* Frees allocated space by given node. */
void QGNode_Free(QGNode* node);


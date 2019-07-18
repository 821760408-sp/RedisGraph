/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../redismodule.h"
#include "../../deps/libcypher-parser/lib/src/cypher-parser.h"

/* Query context, used for concurent query processing. */
typedef struct {
    RedisModuleCtx *ctx;                  // Redis module context.
    RedisModuleBlockedClient *bc;         // Blocked client.
    cypher_parse_result_t *parse_result;  // Parser output.
    char *graphName;                      // Graph ID.
    double tic[2];                        // Timings.
    RedisModuleString **argv;             // Arguments.
    int argc;                             // Argument count.
} CommandCtx;

// Create a new command context.
CommandCtx* CommandCtx_New
(
    RedisModuleCtx *ctx,                  // Redis module context.
    RedisModuleBlockedClient *bc,         // Blocked client.
    cypher_parse_result_t *parse_result,  // Parser output.
    RedisModuleString *graphName,         // Graph ID.
    RedisModuleString **argv,             // Arguments.
    int argc                              // Argument count.
);

// Get Redis module context
RedisModuleCtx* CommandCtx_GetRedisCtx
(
    CommandCtx *qctx
);

// Acquire Redis global lock.
void CommandCtx_ThreadSafeContextLock
(
    const CommandCtx *qctx
);

// Release Redis global lock.
void CommandCtx_ThreadSafeContextUnlock
(
    const CommandCtx *qctx
);

// Free command context.
void CommandCtx_Free
(
    CommandCtx* qctx
);

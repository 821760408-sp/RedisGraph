/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "resultset_formatter.h"
#include "resultset_replynop.h"
#include "resultset_replycompact.h"
#include "resultset_replyverbose.h"

typedef enum {
    FORMATTER_NOP = 0,
    FORMATTER_VERBOSE = 1,
    FORMATTER_COMPACT = 2,
} ResultSetFormatterType;

/* Reply formater which does absolutely nothing.
 * used when profiling a query */
static ResultSetFormatter ResultSetNOP = {
    .EmitRecord = ResultSet_EmitNOPRecord,
    .EmitHeader = ResultSet_EmitNOPHeader
};

/* Compact reply formatter, this is the default formatter. */
static ResultSetFormatter ResultSetFormatterCompact = {
    .EmitRecord = ResultSet_EmitCompactRecord,
    .EmitHeader = ResultSet_ReplyWithCompactHeader
};

/* Verbose reply formatter, used when querying via CLI. */
static ResultSetFormatter ResultSetFormatterVerbose = {
    .EmitRecord = ResultSet_EmitVerboseRecord,
    .EmitHeader = ResultSet_ReplyWithVerboseHeader
};

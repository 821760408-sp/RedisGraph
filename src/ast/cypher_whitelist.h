/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "ast.h"

// Build a whitelist containing all AST types currently supported by the module.
void CypherWhitelist_Build(void);

// Check if any entity in the AST is not in the RedisGraph supported whitelist.
AST_Validation CypherWhitelist_ValidateQuery(const cypher_astnode_t *root, char **reason);
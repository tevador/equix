/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#ifndef SOLVER_H
#define SOLVER_H

#include <equix.h>
#include "context.h"

#define EQUIX_STAGE1_MASK ((1ull << 15) - 1)
#define EQUIX_STAGE2_MASK ((1ull << 30) - 1)
#define EQUIX_FULL_MASK ((1ull << 60) - 1)

int equix_solver_solve(hashx_ctx* hash_func, solver_heap* heap, equix_solution output[EQUIX_MAX_SOLS]);

#endif

/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#ifndef CONTEXT_H
#define CONTEXT_H

#include <equix.h>
#include <hashx.h>
#include <hashwx.h>

typedef struct solver_heap solver_heap;

typedef struct equix_ctx {
    union {
        hashx_ctx* hash_v1;
        hashwx_ctx* hash_v2;
    };
    solver_heap* heap;
    equix_ctx_flags flags;
} equix_ctx;

typedef uint64_t solver_hash_func(equix_ctx* ctx, equix_idx index);

#endif

/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#include <stdlib.h>
#include <equix.h>
#include <virtual_memory.h>
#include "context.h"
#include "solver_heap.h"

equix_ctx* equix_alloc(equix_ctx_flags flags) {
	equix_ctx* ctx_failure = NULL;
	equix_ctx* ctx = malloc(sizeof(equix_ctx));
	if (ctx == NULL) {
		goto failure;
	}
	ctx->flags = flags & (EQUIX_CTX_COMPILE | EQUIX_V2);

    if (flags & EQUIX_V2) {
        ctx->hash_v2 = hashwx_alloc(flags & EQUIX_CTX_COMPILE ?
            HASHWX_COMPILED : HASHWX_INTERPRETED);
        if (ctx->hash_v2 == NULL) {
            goto failure;
        }
        if (ctx->hash_v2 == HASHWX_NOTSUPP) {
            ctx_failure = EQUIX_NOTSUPP;
            goto failure;
        }
    }
    else {
        ctx->hash_v1 = hashx_alloc(flags & EQUIX_CTX_COMPILE ?
            HASHX_COMPILED : HASHX_INTERPRETED);
        if (ctx->hash_v1 == NULL) {
            goto failure;
        }
        if (ctx->hash_v1 == HASHX_NOTSUPP) {
            ctx_failure = EQUIX_NOTSUPP;
            goto failure;
        }
    }
	if (flags & EQUIX_CTX_SOLVE) {
		if (flags & EQUIX_CTX_HUGEPAGES) {
			ctx->heap = hashx_vm_alloc_huge(sizeof(solver_heap));
		}
		else {
			ctx->heap = malloc(sizeof(solver_heap));
		}
		if (ctx->heap == NULL) {
			goto failure;
		}
	}
	ctx->flags = flags;
	return ctx;
failure:
	equix_free(ctx);
	return ctx_failure;
}

void equix_free(equix_ctx* ctx) {
	if (ctx != NULL && ctx != EQUIX_NOTSUPP) {
		if (ctx->flags & EQUIX_CTX_SOLVE) {
			if (ctx->flags & EQUIX_CTX_HUGEPAGES) {
				hashx_vm_free(ctx->heap, sizeof(solver_heap));
			}
			else {
				free(ctx->heap);
			}
		}
        if (ctx->flags & EQUIX_V2) {
            hashwx_free(ctx->hash_v2);
        }
        else {
            hashx_free(ctx->hash_v1);
        }
		free(ctx);
	}
}

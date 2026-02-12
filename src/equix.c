/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <equix.h>
#include <hashx.h>
#include <hashwx.h>
#include "context.h"
#include "solver.h"
#include <hashx_endian.h>
#include <blake2.h>

static bool verify_order(const equix_solution* solution) {
    return
        tree_cmp4(&solution->idx[0], &solution->idx[4]) &
        tree_cmp2(&solution->idx[0], &solution->idx[2]) &
        tree_cmp2(&solution->idx[4], &solution->idx[6]) &
        tree_cmp1(&solution->idx[0], &solution->idx[1]) &
        tree_cmp1(&solution->idx[2], &solution->idx[3]) &
        tree_cmp1(&solution->idx[4], &solution->idx[5]) &
        tree_cmp1(&solution->idx[6], &solution->idx[7]);
}

static uint64_t sum_pair(equix_ctx* ctx, solver_hash_func* hash_func, equix_idx left, equix_idx right) {
    uint64_t hash_left = hash_func(ctx, left);
    uint64_t hash_right = hash_func(ctx, right);
    return hash_left + hash_right;
}

static equix_result verify_internal(equix_ctx* ctx, solver_hash_func* hash_func, const equix_solution* solution) {
    uint64_t pair0 = sum_pair(ctx, hash_func, solution->idx[0], solution->idx[1]);
    if (pair0 & EQUIX_STAGE1_MASK) {
        return EQUIX_PARTIAL_SUM;
    }
    uint64_t pair1 = sum_pair(ctx, hash_func, solution->idx[2], solution->idx[3]);
    if (pair1 & EQUIX_STAGE1_MASK) {
        return EQUIX_PARTIAL_SUM;
    }
    uint64_t pair4 = pair0 + pair1;
    if (pair4 & EQUIX_STAGE2_MASK) {
        return EQUIX_PARTIAL_SUM;
    }
    uint64_t pair2 = sum_pair(ctx, hash_func, solution->idx[4], solution->idx[5]);
    if (pair2 & EQUIX_STAGE1_MASK) {
        return EQUIX_PARTIAL_SUM;
    }
    uint64_t pair3 = sum_pair(ctx, hash_func, solution->idx[6], solution->idx[7]);
    if (pair3 & EQUIX_STAGE1_MASK) {
        return EQUIX_PARTIAL_SUM;
    }
    uint64_t pair5 = pair2 + pair3;
    if (pair5 & EQUIX_STAGE2_MASK) {
        return EQUIX_PARTIAL_SUM;
    }
    uint64_t pair6 = pair4 + pair5;
    if (pair6 & EQUIX_FULL_MASK) {
        return EQUIX_FINAL_SUM;
    }
    return EQUIX_OK;
}

static uint64_t hashfunc_v1(equix_ctx* ctx, equix_idx index) {
    char hash[HASHX_SIZE];
    hashx_exec(ctx->hash_v1, index, hash);
    return load64(hash);
}

static uint64_t hashfunc_v2(equix_ctx* ctx, equix_idx index) {
    return hashwx_exec(ctx->hash_v2, index);
}

/* Blake2b params used to generate HashWX instances */
const blake2b_param equix_v2_blake2_params = {
    .digest_length = HASHWX_SEED_SIZE,
    .key_length = 0,
    .fanout = 1,
    .depth = 1,
    .leaf_length = 0,
    .node_offset = 0,
    .node_depth = 0,
    .inner_length = 0,
    .reserved = { 0 },
    .salt = "Equi-X v2",
    .personal = { 0 }
};

static void make_hashwx_ctx(
    equix_ctx* ctx,
    const void* challenge,
    size_t challenge_size)
{
    /* HashWX seed = blake2b(challenge) */
    uint8_t seed[HASHWX_SEED_SIZE];
    blake2b_state hash_state;
    hashx_blake2b_init_param(&hash_state, &equix_v2_blake2_params);
    hashx_blake2b_update(&hash_state, challenge, challenge_size);
    hashx_blake2b_final(&hash_state, &seed, HASHWX_SEED_SIZE);
    hashwx_make(ctx->hash_v2, seed);
}

int equix_solve(
    equix_ctx* ctx,
    const void* challenge,
    size_t challenge_size,
    equix_solution output[EQUIX_MAX_SOLS])
{
    assert(ctx != NULL);
    assert(challenge != NULL || challenge_size == 0);
    assert(output != NULL);

    if ((ctx->flags & EQUIX_CTX_SOLVE) == 0) {
        return 0;
    }

    if ((ctx->flags & EQUIX_V2) != 0) {
        make_hashwx_ctx(ctx, challenge, challenge_size);
        return equix_solver_solve(ctx, &hashfunc_v2, output);
    }

    if (!hashx_make(ctx->hash_v1, challenge, challenge_size)) {
        return 0;
    }

    return equix_solver_solve(ctx, &hashfunc_v1, output);
}


equix_result equix_verify(
    equix_ctx* ctx,
    const void* challenge,
    size_t challenge_size,
    const equix_solution* solution)
{
    assert(ctx != NULL);
    assert(challenge != NULL || challenge_size == 0);
    assert(solution != NULL);

    if (!verify_order(solution)) {
        return EQUIX_ORDER;
    }

    if ((ctx->flags & EQUIX_V2) != 0) {
        make_hashwx_ctx(ctx, challenge, challenge_size);
        return verify_internal(ctx, &hashfunc_v2, solution);
    }

    if (!hashx_make(ctx->hash_v1, challenge, challenge_size)) {
        return EQUIX_CHALLENGE;
    }
    return verify_internal(ctx, &hashfunc_v1, solution);
}

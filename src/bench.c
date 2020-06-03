/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <equix.h>
#include <test_utils.h>

typedef struct sols_for_seed {
	equix_solution sols[EQUIX_MAX_SOLS];
	int count;
} sols_for_seed;

static void print_solution(int nonce, const equix_solution* sol) {
	output_hex((char*)&nonce, sizeof(nonce));
	printf(" : { ");
	for (int idx = 0; idx < EQUIX_NUM_IDX; ++idx) {
		printf("%#06x%s", sol->idx[idx],
			idx != EQUIX_NUM_IDX - 1 ? ", " : "");
	}
	printf(" }\n");
}

static const char* result_names[] = {
	"OK",
	"Invalid nonce",
	"Indices out of order",
	"Nonzero partial sum",
	"Nonzero final sum"
};

int main(int argc, char** argv) {
	int nonces, start;
	bool interpret, huge_pages, print_sols;
	read_int_option("--nonces", argc, argv, &nonces, 500);
	read_int_option("--start", argc, argv, &start, 0);
	read_option("--interpret", argc, argv, &interpret);
	read_option("--hugepages", argc, argv, &huge_pages);
	read_option("--sols", argc, argv, &print_sols);
	equix_ctx_flags flags = EQUIX_CTX_SOLVE;
	if (!interpret) {
		flags |= EQUIX_CTX_COMPILE;
	}
	if (huge_pages) {
		flags |= EQUIX_CTX_HUGEPAGES;
	}
	equix_ctx* ctx = equix_alloc(flags);
	if (ctx == NULL) {
		printf("Error: memory allocation failure\n");
		return 1;
	}
	if (ctx == EQUIX_NOTSUPP) {
		printf("Error: not supported\n");
		return 1;
	}
	sols_for_seed* bench = malloc(nonces * sizeof(sols_for_seed));
	if (bench == NULL) {
		printf("Error: memory allocation failure\n");
		return 1;
	}
	printf("Solving nonces %i-%i (interpret: %i, hugepages: %i) ...\n", start, start + nonces - 1, interpret, huge_pages);
	int total_sols = 0;
	clock_t clock_start, clock_end;
	clock_start = clock();
	for (int i = 0; i < nonces; ++i) {
		int nonce = start + i;
		int count = equix_solve(ctx, &nonce, sizeof(nonce), bench[i].sols);
		total_sols += count;
		bench[i].count = count;
	}
	clock_end = clock();
	printf("%f solutions/nonce\n", total_sols / (double)nonces);
	printf("%f solutions/sec.\n", total_sols / ((clock_end - clock_start) / (double)CLOCKS_PER_SEC));
	if (print_sols) {
		for (int i = 0; i < nonces; ++i) {
			for (int sol = 0; sol < bench[i].count; ++sol) {
				print_solution(start + i, &bench[i].sols[sol]);
			}
		}
	}
	clock_start = clock();
	for (int i = 0; i < nonces; ++i) {
		for (int sol = 0; sol < bench[i].count; ++sol) {
			int nonce = start + i;
			equix_result result = equix_verify(ctx, &nonce, sizeof(nonce), &bench[i].sols[sol]);
			if (result != EQUIX_OK) {
				printf("Invalid solution (%s):\n", result_names[result]);
				print_solution(nonce, &bench[i].sols[sol]);
			}
		}
	}
	clock_end = clock();
	printf("%f verifications/sec.\n", total_sols / ((clock_end - clock_start) / (double)CLOCKS_PER_SEC));
	free(bench);
	return 0;
}

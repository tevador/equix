/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <equix.h>

typedef struct sols_for_seed {
	equix_solution sols[EQUIX_MAX_SOLS];
	int count;
} sols_for_seed;

inline static void read_option(const char* option, int argc, char** argv, bool* out) {
	for (int i = 0; i < argc; ++i) {
		if (strcmp(argv[i], option) == 0) {
			*out = true;
			return;
		}
	}
	*out = false;
}

inline static void read_int_option(const char* option, int argc, char** argv, int* out, int default_val) {
	for (int i = 0; i < argc - 1; ++i) {
		if (strcmp(argv[i], option) == 0 && (*out = atoi(argv[i + 1])) > 0) {
			return;
		}
	}
	*out = default_val;
}

int main(int argc, char** argv) {
	int nonces;
	bool interpret, huge_pages;
	read_int_option("--nonces", argc, argv, &nonces, 500);
	read_option("--interpret", argc, argv, &interpret);
	read_option("--hugepages", argc, argv, &huge_pages);
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
	printf("Solving %i nonces (interpret: %i, hugepages: %i) ...\n", nonces, interpret, huge_pages);
	int total_sols = 0;
	clock_t clock_start, clock_end;
	clock_start = clock();
	for (int nonce = 0; nonce < nonces; ++nonce) {
		int count = equix_solve(ctx, &nonce, sizeof(nonce), bench[nonce].sols);
		total_sols += count;
		bench[nonce].count = count;
	}
	clock_end = clock();
	printf("%f solutions/nonce\n", total_sols / (double)nonces);
	printf("%f solutions/sec.\n", total_sols / ((clock_end - clock_start) / (double)CLOCKS_PER_SEC));
	clock_start = clock();
	int result = 0;
	for (int nonce = 0; nonce < nonces; ++nonce) {
		for (int sol = 0; sol < bench[nonce].count; ++sol) {
			result |= equix_verify(ctx, &nonce, sizeof(nonce), &bench[nonce].sols[sol]);
		}
	}
	clock_end = clock();
	printf("%f verifications/sec.\n", total_sols / ((clock_end - clock_start) / (double)CLOCKS_PER_SEC));
	free(bench);
	return result;
}

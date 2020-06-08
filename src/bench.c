/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <equix.h>
#include <test_utils.h>
#include <hashx_thread.h>

typedef struct solver_output {
	equix_solution sols[EQUIX_MAX_SOLS];
	int count;
} solver_output;

typedef struct worker_job {
	int id;
	hashx_thread thread;
	equix_ctx* ctx;
	int64_t total_sols;
	int start;
	int step;
	int end;
	solver_output* output;
} worker_job;

static hashx_thread_retval worker(void* args) {
	worker_job* job = (worker_job*)args;
	job->total_sols = 0;
	solver_output* outptr = job->output;
	for (int seed = job->start; seed < job->end; seed += job->step) {
		int count = equix_solve(job->ctx, &seed, sizeof(seed), outptr->sols);
		outptr->count = count;
		job->total_sols += count;
		outptr++;
	}
	return HASHX_THREAD_SUCCESS;
}

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

static void print_help(char* executable) {
	printf("Usage: %s [OPTIONS]\n", executable);
	printf("Supported options:\n");
	printf("  --help        show this message\n");
	printf("  --nonces N    solve N nonces (default: N=500)\n");
	printf("  --start S     start with nonce S (default: S=0)\n");
	printf("  --threads T   use T threads (default: T=1)\n");
	printf("  --interpret   use HashX interpreter\n");
	printf("  --hugepages   use hugepages\n");
	printf("  --sols        print all solutions\n");
}

int main(int argc, char** argv) {
	int nonces, start, threads;
	bool interpret, huge_pages, print_sols, help;
	read_option("--help", argc, argv, &help);
	if (help) {
		print_help(argv[0]);
		return 0;
	}
	read_int_option("--nonces", argc, argv, &nonces, 500);
	read_int_option("--start", argc, argv, &start, 0);
	read_option("--interpret", argc, argv, &interpret);
	read_option("--hugepages", argc, argv, &huge_pages);
	read_option("--sols", argc, argv, &print_sols);
	read_int_option("--threads", argc, argv, &threads, 1);
	equix_ctx_flags flags = EQUIX_CTX_SOLVE;
	if (!interpret) {
		flags |= EQUIX_CTX_COMPILE;
	}
	if (huge_pages) {
		flags |= EQUIX_CTX_HUGEPAGES;
	}
	worker_job* jobs = malloc(sizeof(worker_job) * threads);
	if (jobs == NULL) {
		printf("Error: memory allocation failure\n");
		return 1;
	}
	int per_thread = (nonces + threads - 1) / threads;
	for (int thd = 0; thd < threads; ++thd) {
		jobs[thd].ctx = equix_alloc(flags);
		if (jobs[thd].ctx == NULL) {
			printf("Error: memory allocation failure\n");
			return 1;
		}
		if (jobs[thd].ctx == EQUIX_NOTSUPP) {
			printf("Error: not supported. Try with --interpret\n");
			return 1;
		}
		jobs[thd].id = thd;
		jobs[thd].start = start + thd;
		jobs[thd].step = threads;
		jobs[thd].end = start + nonces;
		jobs[thd].output = malloc(sizeof(solver_output) * per_thread);
		if (jobs[thd].output == NULL) {
			printf("Error: memory allocation failure\n");
			return 1;
		}
	}
	printf("Solving nonces %i-%i (interpret: %i, hugepages: %i, threads: %i) ...\n", start, start + nonces - 1, interpret, huge_pages, threads);
	int total_sols = 0;
	clock_t clock_start, clock_end;
	clock_start = clock();
	if (threads > 1) {
		for (int thd = 0; thd < threads; ++thd) {
			jobs[thd].thread = hashx_thread_create(&worker, &jobs[thd]);
		}
		for (int thd = 0; thd < threads; ++thd) {
			hashx_thread_join(jobs[thd].thread);
		}
	}
	else {
		worker(jobs);
	}
	clock_end = clock();
	for (int thd = 0; thd < threads; ++thd) {
		total_sols += jobs[thd].total_sols;
	}
	double elapsed = (clock_end - clock_start) / (double)CLOCKS_PER_SEC;
#ifndef _MSC_VER
	elapsed /= threads;
#endif
	printf("%f solutions/nonce\n", total_sols / (double)nonces);
	printf("%f solutions/sec. (%i thread%s)\n", total_sols / elapsed, threads, threads > 1 ? "s" : "");
	if (print_sols) {
		for (int thd = 0; thd < threads; ++thd) {
			worker_job* job = &jobs[thd];
			solver_output* outptr = job->output;
			for (int seed = job->start; seed < job->end; seed += job->step) {
				for (int sol = 0; sol < outptr->count; ++sol) {
					print_solution(seed, &outptr->sols[sol]);
				}
				outptr++;
			}
		}
	}
	clock_start = clock();
	for (int thd = 0; thd < threads; ++thd) {
		worker_job* job = &jobs[thd];
		solver_output* outptr = job->output;
		for (int seed = job->start; seed < job->end; seed += job->step) {
			for (int sol = 0; sol < outptr->count; ++sol) {
				equix_result result = equix_verify(job->ctx, &seed, sizeof(seed), &outptr->sols[sol]);
				if (result != EQUIX_OK) {
					printf("Invalid solution (%s):\n", result_names[result]);
					print_solution(seed, &outptr->sols[sol]);
				}
			}
			outptr++;
		}
	}
	clock_end = clock();
	printf("%f verifications/sec. (1 thread)\n", total_sols / ((clock_end - clock_start) / (double)CLOCKS_PER_SEC));
	for (int thd = 0; thd < threads; ++thd) {
		free(jobs[thd].output);
	}
	free(jobs);
	return 0;
}

#ifndef TEST_H
#define TEST_H

#include <stdbool.h>
#include <stdlib.h>

void fast_barriers(void (*barrier)(void),
                   size_t num_barriers,
                   struct timeval *arrive,
                   struct timeval *leave);

void slow_barriers(void (*barrier)(void),
                   size_t num_barriers,
                   struct timeval *arrive,
                   struct timeval *leave);

void print_json_results(size_t total_barriers,
                        int exec_id,
                        struct timeval *arrive,
                        struct timeval *leave);

void run_test_harness(void (*barrier)(void),
                      int rank,
                      unsigned int num_barriers,
                      bool fast);
#endif

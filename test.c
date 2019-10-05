#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "test.h"

#define MICROSECOND 1000000L

void fast_barriers(void (*barrier)(void),
                   size_t num_barriers,
                   struct timeval *arrive,
                   struct timeval *leave) {
    /* Time the fast barriers. */
    struct timeval start;
    struct timeval end;

    /* Start the timer. */
    gettimeofday(&start, NULL);

    /* Fast barriers. */
    for (unsigned int b = 0; b < num_barriers; ++b) {
        gettimeofday(&arrive[b], NULL);
        barrier();
        gettimeofday(&leave[b], NULL);
    }

    /* Get the end timer. */
    gettimeofday(&end, NULL);

    /* Print timing result. */
    long delta_sec = end.tv_sec - start.tv_sec;
    long delta_usec = end.tv_usec - start.tv_usec;
    long delta = delta_sec * MICROSECOND + delta_usec;

    printf("%lu fast barriers took %lu.%06lu s\n",
           num_barriers, delta / MICROSECOND, delta % MICROSECOND);
}

void slow_barriers(void (*barrier)(void),
                   size_t num_barriers,
                   struct timeval *arrive,
                   struct timeval *leave) {
    /* Initialise random number generator. */
    srand(time(NULL));

    /* slow barriers. */
    for (unsigned int b = 0; b < num_barriers; ++b) {
        sleep(rand() % 5);
        gettimeofday(&arrive[b], NULL);
        barrier();
        gettimeofday(&leave[b], NULL);
    }
}

void print_json_results(size_t total_barriers,
                        int exec_id,
                        struct timeval *arrive,
                        struct timeval *leave) {
    /* Create file name for results. */
    char filename[80];
    snprintf(filename, 80, "results/test_%d.json", exec_id);
    FILE *fd = fopen(filename, "w");

    /* Print the results to the file. */
    fprintf(fd, "{\n  \"barriers\": [\n");

    for (unsigned int b = 0; b < total_barriers; ++b) {
        fprintf(fd, "    {\n");

        fprintf(fd, "      \"arr\": {\"sec\":%ld, \"usec\":%ld},\n",
                arrive[b].tv_sec, arrive[b].tv_usec);

        fprintf(fd, "      \"dep\": {\"sec\":%ld, \"usec\":%ld}\n",
                leave[b].tv_sec, leave[b].tv_usec);

        if ((b+1) != total_barriers) {
            fprintf(fd, "    },\n");
        } else {
            fprintf(fd, "    }\n");
        }
    }

    fprintf(fd, "  ]\n}\n");
}

void run_test_harness(void (*barrier)(void),
                      int rank,
                      unsigned int num_barriers,
                      bool fast) {
    /* Initialise time structs. */
    size_t total_barriers = fast ? num_barriers : 2 * num_barriers;
    struct timeval *arrive = calloc(total_barriers, sizeof(struct timeval));
    struct timeval *leave = calloc(total_barriers, sizeof(struct timeval));

    /* Always do the fast tests. */
    fast_barriers(barrier, num_barriers, arrive, leave);

    /* If the fast option is not set also do the slow barriers. */
    if (!fast) {
        slow_barriers(barrier,
                      num_barriers,
                      &arrive[num_barriers],
                      &leave[num_barriers]);
    }

    /* Print results to the file (unless we asked for more than 100,000
     * barriers. */
    if (num_barriers < 100000) {
        print_json_results(total_barriers, rank, arrive, leave);
    }

    /* Free resources. */
    free(arrive);
    free(leave);
}

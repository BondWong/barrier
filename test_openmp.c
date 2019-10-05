#include <stdbool.h>
#include <stdio.h>
#include <omp.h>

#include "test.h"
#include "gtmp.h"

int main(int argc, char **argv)
{
    /* Get the number of barriers to test. */
    if (argc < 2) {
        printf("usage: %s <threads> <num_barriers>\n", argv[0]);
        return -1;
    }

    /* First argument is number of threads. */
    unsigned int num_threads = atoi(argv[1]);

    /* Second argument is number of barriers. */
    unsigned int num_barriers = atoi(argv[2]);

    /* Any other argument means only do fast tests. */
    bool fast = (argc > 3);

    /* MPI parameters and init. */
    omp_set_dynamic(0);
    if (omp_get_dynamic()) {
        printf("Warning: dynamic adjustment of threads has been set\n");
    }
    omp_set_num_threads(num_threads);

    /* Local MPI init. */
    gtmp_init(num_threads);

    /* Run the tests. */
#pragma omp parallel
    {
        int rank = omp_get_thread_num();
        run_test_harness(gtmp_barrier, rank, num_barriers, fast);
    }

    /* Close local MPI. */
    gtmp_finalize();

    return 0;
}

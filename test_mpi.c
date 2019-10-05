#include <stdbool.h>
#include <stdio.h>
#include <mpi.h>

#include "test.h"
#include "gtmpi.h"

int main(int argc, char **argv)
{
    /* Get the number of barriers to test. */
    if (argc < 2) {
        printf("usage: %s <num_barriers>\n", argv[0]);
        return -1;
    }

    /* First argument is number of barriers. */
    unsigned int num_barriers = atoi(argv[1]);

    /* Any other argument means only do fast tests. */
    bool fast = (argc > 2);

    /* MPI parameters and init. */
    int rank, num_procs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* Local MPI init. */
    gtmpi_init(num_procs);

    /* Run the tests. */
    run_test_harness(gtmpi_barrier, rank, num_barriers, fast);

    /* Close local MPI. */
    gtmpi_finalize();

    /* Close MPI. */
    MPI_Finalize();

    return 0;
}

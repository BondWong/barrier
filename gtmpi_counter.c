#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include "gtmpi.h"

/*
    From the MCS Paper: A sense-reversing centralized barrier

    shared count : integer := P
    shared sense : Boolean := true
    processor private local_sense : Boolean := true

    procedure central_barrier
        local_sense := not local_sense // each processor toggles its own sense
	if fetch_and_decrement (&count) = 1
	    count := P
	    sense := local_sense // last processor toggles global sense
        else
           repeat until sense = local_sense
*/

// change 1: do not need to store status from every sender
// static MPI_Status* status_array;
static int P;

void gtmpi_init(int num_threads) {
  P = num_threads;
}

void gtmpi_barrier(){
  int vpid, i;
  MPI_Status status;
  MPI_Comm_rank(MPI_COMM_WORLD, &vpid);

  // change 2: the last one signal everyone while the rest wait for the signal (similar to shared memory version)
  // this can reduce number of message sending from O(n^2) to O(n)
  if (vpid == P - 1) {
    for(i = 0; i < P - 1; i++) MPI_Recv(NULL, 0, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
    for(i = 0; i < P - 1; i++) MPI_Send(NULL, 0, MPI_INT, i, 1, MPI_COMM_WORLD);
  } else {
    MPI_Send(NULL, 0, MPI_INT, P - 1, 1, MPI_COMM_WORLD);
    MPI_Recv(NULL, 0, MPI_INT, P - 1, 1, MPI_COMM_WORLD, &status);
  }
}
// change 3: no need to do anything
void gtmpi_finalize(){}

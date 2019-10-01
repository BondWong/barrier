#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "gtmpi.h"

/*
    From the MCS Paper: The scalable, distributed dissemination barrier with only local spinning.

    type flags = record
        myflags : array [0..1] of array [0..LogP - 1] of Boolean
	partnerflags : array [0..1] of array [0..LogP - 1] of ^Boolean

    processor private parity : integer := 0
    processor private sense : Boolean := true
    processor private localflags : ^flags

    shared allnodes : array [0..P-1] of flags
        //allnodes[i] is allocated in shared memory
	//locally accessible to processor i

    //on processor i, localflags points to allnodes[i]
    //initially allnodes[i].myflags[r][k] is false for all i, r, k
    //if j = (i+2^k) mod P, then for r = 0 , 1:
    //    allnodes[i].partnerflags[r][k] points to allnodes[j].myflags[r][k]

    procedure dissemination_barrier
        for instance : integer :0 to LogP-1
	    localflags^.partnerflags[parity][instance]^ := sense
	    repeat until localflags^.myflags[parity][instance] = sense
	if parity = 1
	    sense := not sense
	parity := 1 - parity
*/

static int P, logP;

int log2(int P) {
	int res = 1;
	int y = 2;
	while (y < P) {
		y *= 2;
		res++;
	}

	return res;
}

void gtmpi_init(int num_threads){
	P = num_threads;
	logP = log2(P);
}

void gtmpi_barrier(){
	int i;
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &i);

	int k;
	for (k = 0; k < logP; k++) {
		int j = (i + (1 << k)) % P; // partner j
		int l = (i - (1 << k) + P) % P; // l's partner is i
		/*
			do not need parity. partity's nature is to form a loop as the following:
			[0, 0] => [1, 0] => [1, 1] (flip sense) => [0, 1] => [0, 0] (flip sense again)
			what really happens is i signals its partner j and waits for l whose partner is i
			therefore, i didn't implement parity here
		 */
		MPI_Send(NULL, 0, MPI_INT, j, 1, MPI_COMM_WORLD); // signals partner
    MPI_Recv(NULL, 0, MPI_INT, l, 1, MPI_COMM_WORLD, &status); // wait for notification
	}
}

void gtmpi_finalize(){

}

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "gtmpi.h"

/*
    From the MCS Paper: A scalable, distributed tournament barrier with only local spinning
    type round_t = record
        role : (winner, loser, bye, champion, dropout)
	opponent : ^Boolean
	flag : Boolean
    shared rounds : array [0..P-1][0..LogP] of round_t
        // row vpid of rounds is allocated in shared memory
	// locally accessible to processor vpid
    processor private sense : Boolean := true
    processor private vpid : integer // a unique virtual processor index
    //initially
    //    rounds[i][k].flag = false for all i,k
    //rounds[i][k].role =
    //    winner if k > 0, i mod 2^k = 0, i + 2^(k-1) < P , and 2^k < P
    //    bye if k > 0, i mode 2^k = 0, and i + 2^(k-1) >= P
    //    loser if k > 0 and i mode 2^k = 2^(k-1)
    //    champion if k > 0, i = 0, and 2^k >= P
    //    dropout if k = 0
    //    unused otherwise; value immaterial
    //rounds[i][k].opponent points to
    //    round[i-2^(k-1)][k].flag if rounds[i][k].role = loser
    //    round[i+2^(k-1)][k].flag if rounds[i][k].role = winner or champion
    //    unused otherwise; value immaterial
    procedure tournament_barrier
        round : integer := 1
	loop   //arrival
	    case rounds[vpid][round].role of
	        loser:
	            rounds[vpid][round].opponent^ :=  sense
		    repeat until rounds[vpid][round].flag = sense
		    exit loop
   	        winner:
	            repeat until rounds[vpid][round].flag = sense
		bye:  //do nothing
		champion:
	            repeat until rounds[vpid][round].flag = sense
		    rounds[vpid][round].opponent^ := sense
		    exit loop
		dropout: // impossible
	    round := round + 1
	loop  // wakeup
	    round := round - 1
	    case rounds[vpid][round].role of
	        loser: // impossible
		winner:
		    rounds[vpid[round].opponent^ := sense
		bye: // do nothing
		champion: // impossible
		dropout:
		    exit loop
	sense := not sense
*/

// type round_t = record
//     role : (winner, loser, bye, champion, dropout)
//     opponent : ^Boolean
//     flag : Boolean

int log2(int P) {
	int res = 1;
	int y = 2;
	while (y < P) {
		y *= 2;
		res++;
	}

	return res;
}

int pow2(int k) {
	return 1 << k;
}

enum role{ WINNER, LOSER, BYE, CHAMPION, DROPOUT, UNUSED };

struct round {
  enum role role_type;
  int opponent; // opponent is the vpid
  // do not need flag here since there is not shared value changed
};

static int P, logP;
struct round **rounds;

void gtmpi_init(int num_threads){
  P = num_threads;
  logP = log2(P);
  rounds = malloc(sizeof(struct round *) * P);
  int i;
  for (i = 0; i < P; i++) {
    rounds[i] = malloc(sizeof(struct round) * (logP + 1)); // shared rounds : array [0..P-1][0..LogP] of round_t
    int k;
    for (k = 0; k < logP + 1; k++) {
      //    winner if k > 0, i mod 2^k = 0, i + 2^(k-1) < P , and 2^k < P
      if(k > 0 && i % pow2(k) == 0 && i + pow2(k - 1) < P && pow2(k) < P) rounds[i][k].role_type = WINNER;
      //    bye if k > 0, i mode 2^k = 0, and i + 2^(k-1) >= P
      else if(k > 0 && i % pow2(k) == 0 && i + pow2(k - 1) >= P) rounds[i][k].role_type = BYE;
      //    loser if k > 0 and i mode 2^k = 2^(k-1)
      else if(k > 0 && (i % pow2(k)) == pow2(k - 1)) rounds[i][k].role_type = LOSER;
      //    champion if k > 0, i = 0, and 2^k >= P
      else if(k > 0 && i == 0 && pow2(k) >= P) rounds[i][k].role_type = CHAMPION;
      //    dropout if k = 0
      else if(k == 0) rounds[i][k].role_type = DROPOUT;
      //    unused otherwise; value immaterial
      else rounds[i][k].role_type = UNUSED;

      // round[i-2^(k-1)][k].flag if rounds[i][k].role = loser
      if (rounds[i][k].role_type == LOSER) rounds[i][k].opponent = i - pow2(k - 1);
      // round[i+2^(k-1)][k].flag if rounds[i][k].role = winner or champion
      else if (rounds[i][k].role_type == WINNER || rounds[i][k].role_type == CHAMPION) rounds[i][k].opponent = i + pow2(k - 1);
      // unused otherwise; value immaterial
      else rounds[i][k].opponent = -1;
    }
  }
}

void gtmpi_barrier(){
	MPI_Status status;
  int i;
  MPI_Comm_rank(MPI_COMM_WORLD, &i);

  int k;
	for (k = 1; k <= logP; k++) {
    // k starts with 1 guarantee role of dropout is impossible
		switch(rounds[i][k].role_type) {
	    // loser:
      //   rounds[vpid][round].opponent^ :=  sense
		  //   repeat until rounds[vpid][round].flag = sense
			case LOSER:
				MPI_Send(NULL, 0, MPI_INT, rounds[i][k].opponent, 1, MPI_COMM_WORLD);
				MPI_Recv(NULL, 0, MPI_INT, rounds[i][k].opponent, 1, MPI_COMM_WORLD, &status);
				break;
      case BYE:
        break;
      // winner:
      //   repeat until rounds[vpid][round].flag = sense
			case WINNER:
				MPI_Recv(NULL, 0, MPI_INT, rounds[i][k].opponent, 1, MPI_COMM_WORLD, &status);
				break;
      // champion:
  	  //   repeat until rounds[vpid][round].flag = sense
  		//   rounds[vpid][round].opponent^ := sense
			case CHAMPION:
				MPI_Recv(NULL, 0, MPI_INT, rounds[i][k].opponent, 1, MPI_COMM_WORLD, &status);
				MPI_Send(NULL, 0, MPI_INT, rounds[i][k].opponent, 1, MPI_COMM_WORLD);
				break;
			default:
				break;
		}
	}

  // winner: rounds[vpid[round].opponent^ := sense
  // bye: do nothing
  // champion: impossible
  // dropout: exit loop
  for (k = logP - 1; k >= 0; k--) {
    switch (rounds[i][k].role_type) {
      case WINNER:
        MPI_Send(NULL, 0, MPI_INT, rounds[i][k].opponent, 1, MPI_COMM_WORLD);
        break;
      default:
        break;
    }
	}

  // no need to flip sense for next barrier
  // sense := not sense
}

void gtmpi_finalize(){
  free(rounds);
}

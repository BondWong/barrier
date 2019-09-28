#include <omp.h>
#include "gtmp.h"

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

static int cnt;
static int sense;
static int *local_sense;

void gtmp_init(int num_threads) {
  cnt = num_threads;
  sense = 1;
  local_sense = malloc(cnt * sizeof(int));
  int i;
  for (i = 0; i < cnt; i++) local_sense[i] = 1;
}

void gtmp_barrier() {
  // toggle its own sense
  int cur_thread_num = omp_get_thread_num();
  local_sense[cur_thread_num] = ! local_sense[cur_thread_num];
  int cached_local_sense = local_sense[cur_thread_num];

  // last processor toggles global sense
  if(__sync_fetch_and_sub(&cnt, 1) == 1) {
    cnt = omp_get_num_threads();
    sense = cached_local_sense;
  }
  else while(cached_local_sense != sense);
}

void gtmp_finalize() {
  free(local_sense);
}

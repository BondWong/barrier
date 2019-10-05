#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include "gtmp.h"

/*
    From the MCS Paper: A scalable, distributed tree-based barrier with only local spinning.

    type treenode = record
        parentsense : Boolean
	parentpointer : ^Boolean
	childpointers : array [0..1] of ^Boolean
	havechild : array [0..3] of Boolean
	childnotready : array [0..3] of Boolean
	dummy : Boolean //pseudo-data

    shared nodes : array [0..P-1] of treenode
        // nodes[vpid] is allocated in shared memory
        // locally accessible to processor vpid
    processor private vpid : integer // a unique virtual processor index
    processor private sense : Boolean

    // on processor i, sense is initially true
    // in nodes[i]:
    //    havechild[j] = true if 4 * i + j + 1 < P; otherwise false
    //    parentpointer = &nodes[floor((i-1)/4].childnotready[(i-1) mod 4],
    //        or dummy if i = 0
    //    childpointers[0] = &nodes[2*i+1].parentsense, or &dummy if 2*i+1 >= P
    //    childpointers[1] = &nodes[2*i+2].parentsense, or &dummy if 2*i+2 >= P
    //    initially childnotready = havechild and parentsense = false

    procedure tree_barrier
        with nodes[vpid] do
	    repeat until childnotready = {false, false, false, false}
	    childnotready := havechild //prepare for next barrier
	    parentpointer^ := false //let parent know I'm ready
	    // if not root, wait until my parent signals wakeup
	    if vpid != 0
	        repeat until parentsense = sense
	    // signal children in wakeup tree
	    childpointers[0]^ := sense
	    childpointers[1]^ := sense
	    sense := not sense
*/

// type treenode = record
// 		parentsense : Boolean
// parentpointer : ^Boolean
// childpointers : array [0..1] of ^Boolean
// havechild : array [0..3] of Boolean
// childnotready : array [0..3] of Boolean

struct treenode {
	int parentsense;
	int* parentpointer;
	int* childpointers[2];
	int havechild[4];
	int childnotready[4];

	int sense; // processor private sense : Boolean
};

// dummy : Boolean //pseudo-data
static int dummy = -1;
static struct treenode * records;

void gtmp_init(int num_threads) {
	records = malloc(sizeof(struct treenode) * num_threads);
	int i, j;
	for (i = 0; i < num_threads; i++) {
		// initially parentsense = false sense is initially true
		struct treenode node = {0, &dummy, {0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 1};
		records[i] = node;
		for (j = 0; j < 4; j++) {
			// havechild[j] = true if 4 * i + j + 1 < P; otherwise false
			if (4 * i + j + 1 < num_threads) records[i].havechild[j] = 1;
			else records[i].havechild[j] = 0;
			records[i].childnotready[j] = records[i].havechild[j]; // childnotready = havechild
		}

		// parentpointer = &nodes[floor((i-1)/4].childnotready[(i-1) mod 4] or dummy if i = 0
		if(i == 0) records[i].parentpointer = &dummy;
		else records[i].parentpointer = &(records[(i - 1) / 4].childnotready[(i - 1) % 4]);

		// childpointers[0] = &nodes[2*i+1].parentsense, or &dummy if 2*i+1 >= P
		// childpointers[1] = &nodes[2*i+2].parentsense, or &dummy if 2*i+2 >= P
		for (j = 0; j < 2; j++) {
			if (2 * i + j >= num_threads) records[i].childpointers[j] = &dummy;
			else records[i].childpointers[j] = &(records[2 * i + j].parentsense);
		}
	}
}

void gtmp_barrier() {
	int i = omp_get_thread_num();

	// 	repeat until childnotready = {false, false, false, false}
	int* childnotready = records[i].childnotready;
	while(childnotready[0]);
	while(childnotready[1]);
	while(childnotready[2]);
	while(childnotready[3]);

	// 	childnotready := havechild //prepare for next barrier
	int j;
	for (j = 0; j < 4; j++) records[i].childnotready[j] = records[i].havechild[j];

	// 	parentpointer^ := false //let parent know I'm ready
	// 	// if not root, wait until my parent signals wakeup
	// 	if vpid != 0
	// 			repeat until parentsense = sense
	if (i != 0) {
		printf("thread[%d] notify parent I'm ready\n", i); fflush(stdout);
		*(records[i].parentpointer) = 0;
		printf("thread[%d] wait for signal\n", i); fflush(stdout);
		printf("thread[%d] sense %d\n", records[i].sense); fflush(stdout);
		printf("thread[%d] parentsense %d\n", records[i].parentsense); fflush(stdout);
		while(records[i].parentsense != records[i].sense);
	} else {
		// 	// signal children in wakeup tree
		// 	childpointers[0]^ := sense
		*(records[i].childpointers[0]) = records[i].sense;
		// 	childpointers[1]^ := sense
		*(records[i].childpointers[1]) = records[i].sense;
		// 	sense := not sense
		printf("thread[%d] reverse sense\n", i); fflush(stdout);
		records[i].sense = !(records[i].sense);
	}
}

void gtmp_finalize() {
	free(records);
}

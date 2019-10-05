#OpenMP Flags Etc.
OMPFLAGS = -g -Wall -fopenmp -DLEVEL1_DCACHE_LINESIZE=`getconf LEVEL1_DCACHE_LINESIZE`
OMPLIBS = -lgomp
CC = gcc

#MPI Flags Etc (may need to customize)
#MPICH = /usr/lib64/openmpi/1.4-gcc
MPIFLAGS = -g -Wall #-I$(MPICH)/include
MPILIBS =
#MPICC = /opt/openmpi-1.4.3-gcc44/bin/mpicc
MPICC = mpicc

HDRS = test.h
SRCS = test.c
OPENMP_SRCS = test_openmp.c $(SRCS)
MPI_SRCS = test_mpi.c $(SRCS)

hello_openmp: hello_openmp.c
	$(CC) $(OMPFLAGS) -o $@ $^ $(OMPLIBS)

openmp_counter: gtmp_counter.c $(OPENMP_SRCS) $(HDRS)
	$(CC) $(OMPFLAGS) -o $@ $< $(OPENMP_SRCS) $(OMPLIBS)

openmp_mcs: gtmp_mcs.c $(OPENMP_SRCS) $(HDRS)
	$(CC) $(OMPFLAGS) -o $@ $< $(OPENMP_SRCS) $(OMPLIBS)

openmp_tree: gtmp_tree.c $(OPENMP_SRCS) $(HDRS)
	$(CC) $(OMPFLAGS) -o $@ $< $(OPENMP_SRCS) $(OMPLIBS)

hello_mpi: hello_mpi.c
	$(MPICC) $(MPIFLAGS) -o $@ $^ $(MPILIBS)

mpi_counter: gtmpi_counter.c $(MPI_SRCS) $(HDRS)
	$(MPICC) $(MPIFLAGS) -o $@ $< $(MPI_SRCS) $(MPILIBS)

mpi_dissemination: gtmpi_dissemination.c $(MPI_SRCS) $(HDRS)
	$(MPICC) $(MPIFLAGS) -o $@ $< $(MPI_SRCS) $(MPILIBS)

mpi_tournament: gtmpi_tournament.c $(MPI_SRCS) $(HDRS)
	$(MPICC) $(MPIFLAGS) -o $@ $< $(MPI_SRCS) $(MPILIBS)

clean:
	rm -f hello_openmp hello_mpi \
	      openmp_counter openmp_mcs openmp_tree \
	      mpi_counter mpi_dissemination mpi_tournament

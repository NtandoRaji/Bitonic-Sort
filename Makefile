P1 = bitonic
P2 = bitonic_omp
P3 = bitonic_mpi

# Choose either the C compiler or the C++ compiler from the following 2 lines
CC = gcc
CPP = g++
MPICC = mpic++
CFLAGS = -g -Wall
OMPFLAG = -fopenmp
INCFLAG = -I ""

all: $(P1) $(P2) $(P3)

$(P1): $(P1).cpp
	$(CPP) $(CFLAGS) $(OMPFLAG) $(INCFLAG) ./$(P1).cpp -o $(P1)

$(P2): $(P2).cpp
	$(CPP) $(CFLAGS) $(OMPFLAG) $(INCFLAG) ./$(P2).cpp -o $(P2)

$(P3): $(P3).cpp
	$(MPICC) $(CFLAGS) $(INCFLAG) ./$(P3).cpp -o $(P3)

clean:
	rm -vf $(P1) $(P2) $(P3)

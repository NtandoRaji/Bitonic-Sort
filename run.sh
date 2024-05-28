#!/bin/bash

# Check if at least one argument is passed
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <power of 2> <number of processers>"
    echo "Example Usage: $0 64 (2^64 elements) 4 (4 processers)"
    exit 1
fi

echo "------------Bitonic is starting---------------"
./bitonic "$1" |& tee -a terminal.out
echo "-------------Bitonic is done------------------"
echo

echo "------------Bitonic_OMP is starting---------------"
./bitonic_omp "$2" "$1" |& tee -a terminal.out
echo "-------------Bitonic_OMP is done------------------"
echo

echo "------------Bitonic_MPI is starting---------------"
mpiexec -n "$2" ./bitonic_mpi "$1" |& tee -a terminal.out
echo "-------------Bitonic_MPI is done------------------"
echo
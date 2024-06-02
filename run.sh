#!/bin/bash

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <power of 2> <number of processers>"
    echo "Example Usage: $0 64 (2^64 elements) 4 (4 processers)"
    exit 1
fi


echo "------------Bitonic is starting---------------"
./bitonic "$1" |& tee -a bitonic.out

seq_time=$(grep "Sequential (Quicksort) Sort Time" bitonic.out | awk '{print $5}')

echo "-------------Bitonic is done------------------"
echo


echo "------------Bitonic_OMP is starting---------------"
./bitonic_omp "$2" "$1" |& tee -a bitonic_omp.out

omp_time=$(grep "OpenMP Implementation - Parallel Bitonic Sort Time" bitonic_omp.out | awk '{print $11}')
valid_omp_sort=$(grep "Sort Valid?" bitonic_omp.out | awk '{print $3}')

if [[ $valid_omp_sort == "True" ]]
then
    speedup=$(echo "$seq_time/$omp_time" | bc -l)
    speedup=$(printf "%.6f" "$speedup")
    echo -e "Speedup: $speedup\n"
fi

echo "-------------Bitonic_OMP is done------------------"
echo


echo "------------Bitonic_MPI is starting---------------"
mpiexec -np "$2" --oversubscribe ./bitonic_mpi "$1" |& tee -a bitonic_mpi.out

mpi_time=$(grep "MPI Implementation - Parallel Bitonic Sort Time" bitonic_mpi.out | awk '{print $11}')
valid_mpi_sort=$(grep "Sort Valid?" bitonic_mpi.out | awk '{print $3}')

if [[ $valid_mpi_sort == "True" ]]
then
    speedup=$(echo "$seq_time/$mpi_time" | bc -l)
    speedup=$(printf "%.6f" "$speedup")
    echo -e "Speedup: $speedup\n"
fi

echo "-------------Bitonic_MPI is done------------------"
echo

# Remove output file
rm -f bitonic.out bitonic_omp.out bitonic_mpi.out

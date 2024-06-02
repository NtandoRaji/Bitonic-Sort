#include <iostream>
#include <mpi.h>
#include <math.h>
#include <stdlib.h> 
using namespace std;


void generate_data(int* array, int start, int sample_size, int rank);
void print(int* array, int sample_size);
int get_pivot(int* array, int left, int right);
void quick_sort(int *array, int left, int right);
void compare_low(int* array, int sample_size, int rank, int j);
void compare_high(int* array, int sample_size, int rank, int j);
bool is_valid_sort(int* array, int global_size);


int main(int argc, char* argv[])
{   
    int n_processes, rank, sample_size, global_size;
    int *data, *output;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    // Generate array with random values for each process
    global_size = 1 << atoi(argv[1]);
    sample_size = global_size / n_processes;
    
    data = (int*) malloc(sample_size * sizeof(int));
    output = (int*) malloc(global_size * sizeof(int));

    generate_data(data, 0, sample_size, rank);

    // Wait until all processes have a random array
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0){
        start_time = MPI_Wtime();
    }


    // Sort local data sequentially - Quick Sort
    quick_sort(data, 0, sample_size - 1);

    // Sort using Bitonic Sort //
    int cude_dims = log2(n_processes);

    for (int i = 0; i < cude_dims; i++){
        for (int j = i; j >= 0; j--){
            if (((rank >> (i + 1)) % 2 == 0 && (rank >> j) % 2 == 0) || ((rank >> (i + 1)) % 2 != 0 && (rank >> j) % 2 != 0)){
                compare_low(data, sample_size, rank, j);
            }
            else {
                compare_high(data, sample_size, rank, j);
            }
        }
    }

    // Block until all processes have sorted
    MPI_Barrier(MPI_COMM_WORLD);
    
    MPI_Gather(data, sample_size, MPI_INT, output, sample_size, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        end_time = MPI_Wtime();

        bool is_valid = is_valid_sort(output, global_size);

        printf("\nMPI Implementation - Parallel Bitonic Sort Time using %d processers: %f seconds\n", n_processes, end_time - start_time);

        printf("\nSort Valid?: %s\n", is_valid ? "True" : "False");
    }

    // Free up memory
    free(data);
    free(output);
    MPI_Finalize();
    return 0;
}


void generate_data(int* array, int start, int sample_size, int rank)
{   
    srand(42 + rank);
    for(int i = start; i < start + sample_size; i++){
        array[i] = rand() % 40001;
    }
}


void print(int* array, int sample_size)
{
    for (int i = 0; i < sample_size - 1; i++){
        printf("%d ", array[i]);
    }
    printf("%d\n", array[sample_size - 1]);
}


int get_pivot(int* array, int left, int right)
{
    int pivot = array[right];
    int index = left - 1;

    for (int i = left; i <= right; i++){
        if (array[i] >= pivot) continue;

        index++;
        swap(array[index], array[i]);
    }

    swap(array[index + 1], array[right]);
    return index + 1;
}


void quick_sort(int* array, int left, int right)
{
    if (left >= right) return;

    int pivot = get_pivot(array, left, right);

    quick_sort(array, left, pivot - 1);
    quick_sort(array, pivot + 1, right);
}


void compare_low(int* array, int sample_size, int rank, int j) {
    int partner = rank ^ (1 << j);
    int *recv_buffer = (int*) malloc(sample_size * sizeof(int));
    MPI_Sendrecv(array, sample_size, MPI_INT, partner, 0, recv_buffer, sample_size, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int* merged = (int*) malloc(2 * sample_size * sizeof(int));
    int i = 0, k = 0, l = 0;

    while (k < sample_size && l < sample_size) {
        if (array[k] < recv_buffer[l]) {
            merged[i++] = array[k++];
        } else {
            merged[i++] = recv_buffer[l++];
        }
    }

    while (k < sample_size) {
        merged[i++] = array[k++];
    }

    while (l < sample_size) {
        merged[i++] = recv_buffer[l++];
    }

    for (i = 0; i < sample_size; i++) {
        array[i] = merged[i];
    }

    free(recv_buffer);
    free(merged);
}


void compare_high(int* array, int sample_size, int rank, int j) {
    int partner = rank ^ (1 << j);
    int *recv_buffer = (int*) malloc(sample_size * sizeof(int));
    MPI_Sendrecv(array, sample_size, MPI_INT, partner, 0, recv_buffer, sample_size, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int* merged = (int*) malloc(2 * sample_size * sizeof(int));
    int i = 0, k = sample_size - 1, l = sample_size - 1;

    while (k >= 0 && l >= 0) {
        if (array[k] > recv_buffer[l]) {
            merged[i++] = array[k--];
        } else {
            merged[i++] = recv_buffer[l--];
        }
    }

    while (k >= 0) {
        merged[i++] = array[k--];
    }

    while (l >= 0) {
        merged[i++] = recv_buffer[l--];
    }

    for (i = 0; i < sample_size; i++) {
        array[i] = merged[sample_size - 1 - i];
    }

    free(recv_buffer);
    free(merged);
}


bool is_valid_sort(int* array, int global_size)
{
    for (int i = 1; i < global_size - 1; i++) {
        if (array[i] > array[i + 1]) {
            return false;
        }
    }
    return true;
}

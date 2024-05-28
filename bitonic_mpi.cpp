#include <iostream>
#include <mpi.h>
#include <math.h>
#include <stdlib.h> 
using namespace std;


void generate_data(int* array, int start, int sample_size, int rank);
void print(int* array, int sample_size);
int partition(int *array, int left, int right);
void quickSort(int *array, int left, int right);
void compareLow(int* array, int sample_size, int rank, int j);
void compareHigh(int* array, int sample_size, int rank, int j);

int main(int argc, char* argv[])
{   
    int n_processes, rank, sample_size, global_size;
    int *data, *output, *validation;
    double seq_start_time, seq_end_time, mpi_start_time, mpi_end_time;

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
        mpi_start_time = MPI_Wtime();
    }


    // Sort local data sequentially - Quick Sort
    quickSort(data, 0, sample_size - 1);

    // Sort using Bitonic Sort //
    int cude_dims = log2(n_processes);

    for (int i = 0; i < cude_dims; i++){
        for (int j = i; j >= 0; j--){
            if (((rank >> (i + 1)) % 2 == 0 && (rank >> j) % 2 == 0) || ((rank >> (i + 1)) % 2 != 0 && (rank >> j) % 2 != 0)){
                compareLow(data, sample_size, rank, j);
            }
            else {
                compareHigh(data, sample_size, rank, j);
            }
        }
    }

    // Block until all processes have sorted
    MPI_Barrier(MPI_COMM_WORLD);
    
    MPI_Gather(data, sample_size, MPI_INT, output, sample_size, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        mpi_end_time = MPI_Wtime();

        validation = (int*) malloc(global_size * sizeof(int));
        for (int process = 0; process < n_processes; process++){
            generate_data(validation, process * sample_size, sample_size, process);
        }

        seq_start_time = MPI_Wtime();
        quickSort(validation, 0, global_size - 1);
        seq_end_time = MPI_Wtime();

        bool is_valid = true;
        for (int i = 0; i < global_size; i++){
            if (output[i] != validation[i]){
                is_valid = false;
                break;
            }
        }

        printf("\nSort Valid?: %s\n", is_valid ? "True" : "False");
        printf("Sequential Sort Time: %f\n", seq_end_time - seq_start_time);
        printf("MPI Implementation - Parallel Bitonic Sort Time using %d processers: %f\n", n_processes, mpi_end_time - mpi_start_time);
        printf("Speedup: %f\n\n", (seq_end_time - seq_start_time) / (mpi_end_time - mpi_start_time));
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
        array[i] = rand() % sample_size;
    }
}


void print(int* array, int sample_size)
{
    for (int i = 0; i < sample_size - 1; i++){
        printf("%d ", array[i]);
    }
    printf("%d\n", array[sample_size - 1]);
}

int partition(int* array, int left, int right)
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


void quickSort(int* array, int left, int right)
{
    if (left >= right) return;

    int pivot = partition(array, left, right);

    quickSort(array, left, pivot - 1);
    quickSort(array, pivot + 1, right);
}

void compareLow(int* array, int sample_size, int rank, int j) {
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

void compareHigh(int* array, int sample_size, int rank, int j) {
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
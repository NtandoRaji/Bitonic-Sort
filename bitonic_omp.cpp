#include <iostream>
#include <omp.h>
#include <cmath>
#include <cstdlib>
using namespace std;

void generate_data(int* array, int start, int sample_size, int rank);
void print(int* array, int sample_size);
int partition(int* array, int left, int right);
void quickSort(int* array, int left, int right);
void bitonicSort(int* array, int low, int count, int direction);
void bitonicMerge(int* array, int low, int count, int direction);


int main(int argc, char* argv[]) {
    int n_threads, sample_size, global_size;
    int *data, *validation;
    double seq_start_time, seq_end_time, omp_start_time, omp_end_time;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_threads> <power_of_two>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    n_threads = atoi(argv[1]);
    global_size = 1 << atoi(argv[2]);
    sample_size = global_size / n_threads;

    data = (int*) malloc(global_size * sizeof(int));
    validation = (int*) malloc(global_size * sizeof(int));

    for (int process = 0; process < n_threads; process++) {
        generate_data(data, sample_size * process, sample_size, process);
        generate_data(validation, sample_size * process, sample_size, process);
    }

    seq_start_time = omp_get_wtime();
    quickSort(validation, 0, global_size - 1);
    seq_end_time = omp_get_wtime();

    omp_start_time = omp_get_wtime();
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp single
        bitonicSort(data, 0, global_size, 1);
    }
    omp_end_time = omp_get_wtime();

    bool is_valid = true;
    for (int i = 0; i < global_size; i++) {
        if (data[i] != validation[i]) {
            is_valid = false;
            break;
        }
    }

    printf("\nSort Valid?: %s\n", is_valid ? "True" : "False");
    printf("Sequential (Quicksort) Sort Time: %f\n", seq_end_time - seq_start_time);
    printf("OpenMP Implementation - Parallel Bitonic Sort Time using %d threads: %f seconds\n", n_threads, omp_end_time - omp_start_time);
    printf("Speedup: %f\n\n", (seq_end_time - seq_start_time) / (omp_end_time - omp_start_time));

    free(data);
    free(validation);
    return 0;
}

void generate_data(int* array, int start, int sample_size, int rank) {
    srand(42 + rank);
    for (int i = start; i < start + sample_size; i++) {
        array[i] = rand() % sample_size;
    }
}


void print(int* array, int sample_size) {
    for (int i = 0; i < sample_size; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}


int get_pivot(int* array, int left, int right) {
    int pivot = array[right];
    int index = left - 1;

    for (int i = left; i <= right; i++) {
        if (array[i] >= pivot) continue;

        index++;
        swap(array[index], array[i]);
    }

    swap(array[index + 1], array[right]);
    return index + 1;
}


void quickSort(int* array, int left, int right) {
    if (left >= right) return;

    int pivot = get_pivot(array, left, right);

    quickSort(array, left, pivot - 1);
    quickSort(array, pivot + 1, right);
}


void bitonicSort(int* array, int low, int count, int direction)
{
    if (count <= 1) return;

    int k = count / 2;
    #pragma omp task if (k > 512)
    {
        bitonicSort(array, low, k, 1); // Sort in increasing order
    }

    #pragma omp task if (k > 512)
    {
        bitonicSort(array, low + k, k, 0); // Sort in decreasing order
    }

    #pragma omp taskwait
    bitonicMerge(array, low, count, direction);
}


void bitonicMerge(int* array, int low, int count, int direction)
{
    if (count <= 1) return;

    int k = count / 2;

    for (int i = low; i < low + k; i++){
        if (direction == (array[i] > array[k + i])){
            swap(array[i], array[k + i]);
        }
    }

    bitonicMerge(array, low, k, direction);
    bitonicMerge(array, low + k, k, direction);
}
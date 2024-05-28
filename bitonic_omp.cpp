#include <iostream>
#include <omp.h>
#include <cstring>
using namespace std;


void print(int* array, int sample_size);
void generate_data(int* array, int start, int sample_size, int id);
int partition(int* array, int left, int right);
void quickSort(int* array, int left, int right);
void bitonicSort(int* array, int count, int n_threads);
void swap_values(int* array, int i, int j);

int main(int argc, char* argv[])
{   
    int n_threads, global_size;
    int *data, *validation;
    double seq_start_time, seq_end_time, omp_start_time, omp_end_time, seq_time, omp_time;

    n_threads = atoi(argv[1]);
    global_size = 1 << atoi(argv[2]); // Power of 2

    /////////////////////////////////////
    //  Sequential (Quicksort) Sorting //
    /////////////////////////////////////
    validation = (int* ) malloc(global_size * sizeof(int));
    generate_data(validation, 0, global_size, 0);

    seq_start_time = omp_get_wtime();
    quickSort(validation, 0, global_size - 1);
    seq_end_time = omp_get_wtime();
    seq_time = seq_end_time - seq_start_time;

    //////////////////////////////////////
    //  Paralled (Bitonic Sort) Sorting //
    //////////////////////////////////////
    data = (int*) malloc(global_size * sizeof(int));
    generate_data(data, 0, global_size, 0);

    omp_start_time = omp_get_wtime();
    bitonicSort(data, global_size, n_threads); // directions: 1 - ascending, 0 - descending
    omp_end_time = omp_get_wtime();
    omp_time = omp_end_time - omp_start_time;

    bool is_valid = true;
    for (int i = 0; i < global_size; i++){
        if (data[i] != validation[i]){
            is_valid = false;
            break;
        }
    }

    printf("\nSort Valid?: %s\n", is_valid ? "True" : "False");
    printf("Sequential Sort Time: %f\n", seq_time);
    printf("OMP Implementation - Parallel Bitonic Sort Time using %d processers: %f\n", n_threads, omp_time);
    
    printf("Speedup: %f\n\n", seq_time / omp_time);

    return 0;
}


void generate_data(int* array, int start, int sample_size, int id)
{   
    srand(42 + id);
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


void bitonicSort(int* array, int global_size, int n_threads)
{
    int ij;

    for (int k = 2; k <= global_size; k = 2 * k) {
        for (int j = k >> 1; j > 0; j = j >> 1) {
            #pragma omp parallel for private(ij) num_threads(n_threads)
            for (int i = 0; i < global_size; i++) {
                ij = i ^ j;
                if (ij > i) {
                    if (((i & k) == 0 && array[i] > array[ij]) || ((i & k) != 0 && array[i] < array[ij])){
                        swap(array[i], array[ij]);
                    }
                }
            }
        }
    }
}

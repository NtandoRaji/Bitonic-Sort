#include <iostream>
#include <omp.h>
#include <cmath>
#include <cstdlib>
using namespace std;


void generate_data(int* array, int start, int sample_size, int rank);
void print(int* array, int sample_size);
int get_pivot(int* array, int left, int right);
void quick_sort(int* array, int left, int right);
void bitonic_sort(int* array, int count);
bool is_valid_sort(int* array, int global_size);


int main(int argc, char* argv[]) {
    int n_threads, sample_size, global_size;
    int *data;
    double start_time, end_time;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_threads> <power_of_two>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    n_threads = atoi(argv[1]);
    global_size = 1 << atoi(argv[2]);
    sample_size = global_size / n_threads;

    data = (int*) malloc(global_size * sizeof(int));
    for (int process = 0; process < n_threads; process++) {
        generate_data(data, sample_size * process, sample_size, process);
    }

    start_time = omp_get_wtime();
    omp_set_num_threads(n_threads);
    bitonic_sort(data, global_size);
    end_time = omp_get_wtime();

    bool is_valid = is_valid_sort(data, global_size);

    printf("\nOpenMP Implementation - Parallel Bitonic Sort Time using %d threads: %f seconds\n", n_threads, end_time - start_time);
    
    printf("\nSort Valid?: %s\n", is_valid ? "True" : "False");

    free(data);
    return 0;
}


void generate_data(int* array, int start, int sample_size, int rank) {
    srand(42 + rank);
    for (int i = start; i < start + sample_size; i++) {
        array[i] = rand() % 40001;
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


void quick_sort(int* array, int left, int right) {
    if (left >= right) return;

    int pivot = get_pivot(array, left, right);

    quick_sort(array, left, pivot - 1);
    quick_sort(array, pivot + 1, right);
}


void bitonic_sort(int* array, int count) {
    for (int k = 2; k <= count; k = 2 * k) {
        for (int j = k >> 1; j > 0; j = j >> 1) {

            #pragma omp parallel for schedule(static) shared(array, count, k, j)
            for (int i = 0; i < count; i++) {
                int ij = i ^ j;
                
                if (ij > i) {    
                    bool direction = ((i & k) == 0);

                    if ((direction == (array[i] > array[ij])) || (direction != (array[i] < array[ij]))) {
                        swap(array[i], array[ij]);
                    }
                }
            }
        }
    }
}


bool is_valid_sort(int* array, int global_size)
{
    for (int i = 0; i < global_size - 1; i++) {
        if (array[i] > array[i + 1]) {
            return false;
        }
    }
    return true;
}

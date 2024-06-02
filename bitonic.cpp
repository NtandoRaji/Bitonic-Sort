#include <iostream>
#include <stdlib.h>
#include <omp.h>
using namespace std;


void generate_data(int* array, int start, int sample_size);
void print(int* array, int sample_size);
int get_pivot(int* array, int left, int right);
void quick_sort(int* array, int left, int right);


int main(int argc, char* argv[])
{   
    int sample_size = 1 << atoi(argv[1]);
    int* data = (int*) malloc(sample_size * sizeof(int));;
    generate_data(data, 0, sample_size);

    double start_time = omp_get_wtime();
    quick_sort(data, 0, sample_size - 1);
    printf("\nSequential (Quicksort) Sort Time: %f\n\n", omp_get_wtime() - start_time);

    return 0;
}


void generate_data(int* array, int start, int sample_size)
{   
    srand(42);
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
        if (array[i] < pivot){
            index++;
            swap(array[index], array[i]);
        }
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

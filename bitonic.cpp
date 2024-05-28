#include <iostream>
#include <stdlib.h>
using namespace std;


void generate_data(int* array, int start, int sample_size);
void print(int* array, int sample_size);
int partition(int* array, int left, int right);
void quickSort(int* array, int left, int right);


int main(int argc, char* argv[])
{   
    int sample_size = 1 << atoi(argv[1]);
    int* data = (int*) malloc(sample_size * sizeof(int));;
    generate_data(data, 0, sample_size);

    quickSort(data, 0, sample_size - 1);

    return 0;
}


void generate_data(int* array, int start, int sample_size)
{   
    srand(42);
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

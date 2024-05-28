#include <iostream>
#include <omp.h>
using namespace std;


void bitonicMerge(int array[], int low, int count, int direction)
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


void bitonicSort(int array[], int low, int count, int direction)
{
    if (count <= 1) return;

    int k = count / 2;
    
    bitonicSort(array, low, k, 1); // Sort in increasing order
    bitonicSort(array, low + k, k, 0); // Sort in decreasing order

    bitonicMerge(array, low, count, direction);
}


int main(int argc, char* argv[])
{
    return 0;
}
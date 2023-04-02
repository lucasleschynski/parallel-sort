#include <omp.h>
#include <random>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

void print_array(int array[], int length) {
    printf("[");
    for(int i = 0;i<length;i++) {
        i < length-1 ? printf("%d, ", array[i]) : printf("%d", array[i]);
    }
    printf("]\n");
}

// Merges two subarrays of array[] -> array[l..m] and array[m+1..r]
void merge(int array[], int l, int m, int r) {
    int length = r - l + 1;
    int first = l;
    int second = m+1;
    int *fullcopy = new int[length];
    int index = 0;
    while(first <= m && second <= r) {
        if(array[first] < array[second]) {
            fullcopy[index] = array[first];
            first++;
        } else {
            fullcopy[index] = array[second];
            second++;
        }
        index++;
    }
    bool smaller = first > m ? true : false;
    if(smaller) {
        while(second <= r) {
            fullcopy[index] = array[second];
            second++;
            index++;
        }
    } else {
        while(first <= m) {
            fullcopy[index] = array[first];
            first++;
            index++;
        }
    }
    for(int i = 0; i < length; i++) {
        array[l+i] = fullcopy[i];
    }
    delete[] fullcopy;
}

void merge_sort(int array[], int l, int r) {
    if(l >= r) {
        return;
    }

    int m = l + ((r-l) / 2);
    merge_sort(array, l, m);
    merge_sort(array, m + 1, r);
    merge(array, l, m, r);
}

void generate_random_array(int list[], int length) {
    int i;
    srand((unsigned int)time(NULL));
    for(i = 0; i < length; i++) {
        list[i] = rand() % (length*2);
    }
}

int main() {
    // Generate random array for sorting
    int num_elements = 16;
    int random_array[num_elements];
    generate_random_array(random_array, num_elements);
    for(int i = 0;i<num_elements;i++) {
        printf("%d\n", random_array[i]);
    }

    // int split[] = {1,3,5,7,2,4,6,8};
    int split[] = {2,4,6,8,1,3,5,7};
    merge(split, 0, 3, 7);
    print_array(split, 8);

    // Test threads
    int nthreads, tid;
  
    // Begin of parallel region
    #pragma omp parallel private(nthreads, tid)
    {
        // Getting thread number
        tid = omp_get_thread_num();
        printf("Welcome to GFG from thread = %d\n",
               tid);
  
        if (tid == 0) {
  
            // Only master thread does this
            nthreads = omp_get_num_threads();
            printf("Number of threads = %d\n",
                   nthreads);
        }
    }
}
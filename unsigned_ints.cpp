#include <omp.h>
#include <random>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

void print_array(int array[], unsigned int length) {
    printf("[");
    for(int i = 0;i<length;i++) {
        i < length-1 ? printf("%d, ", array[i]) : printf("%d", array[i]);
    }
    printf("]\n");
}

void generate_random_array(unsigned int list[], unsigned int length) {
    unsigned int i;
    srand((unsigned int)time(NULL));
    for(i = 0; i < length; i++) {
        list[i] = rand() % (length*2);
    }
}

void insertion_sort(unsigned int array[], unsigned int l, unsigned int r) {
    unsigned int i, key, j;
    for (i = l+1; i < r+1; i++) {
        key = array[i];
        j = i - 1;
        while (j >= 0 && array[j] > key) {
            array[j + 1] = array[j];
            j = j - 1;
        }
        array[j + 1] = key;
    }
}

// Merges two subarrays of array[] -> array[l..m] and array[m+1..r]
void merge(unsigned int array[], unsigned int l, unsigned int m, unsigned int r) {
    unsigned int length = r - l + 1;
    unsigned int a = l;
    unsigned int b = m+1;
    unsigned int *fullcopy = new unsigned int[length];
    unsigned int i = 0;
    while(a <= m && b <= r) {
        if(array[a] < array[b]) {
            fullcopy[i] = array[a];
            a++;
        } else {
            fullcopy[i] = array[b];
            b++;
        }
        i++;
    }
    bool left_merged = a > m ? true : false;
    if(left_merged) {
        while(b <= r) {
            fullcopy[i] = array[b];
            b++;
            i++;
        }
    } else {
        while(a <= m) {
            fullcopy[i] = array[a];
            a++;
            i++;
        }
    }
    for(int i = 0; i < length; i++) {
        array[l+i] = fullcopy[i];
    }
    delete[] fullcopy;
}

void merge_sort_serial(unsigned int array[], unsigned int l, unsigned int r) {
    if(l < r) {
        if(l-r >= 43) {
            unsigned int m = l + ((r-l) / 2);
            merge_sort_serial(array, l, m);
            merge_sort_serial(array, m + 1, r);
            merge(array, l, m, r);
        } else {
            insertion_sort(array, l, r + 1);
        }
    } else {
        return;
    }
}

void merge_sort_parallel(unsigned int array[], unsigned int l, unsigned int r) {
    if(l < r) {
        if(l-r >= 43) {
            unsigned int m = l + ((r-l) / 2);
            #pragma omp taskgroup 
            {
                #pragma omp task shared(array) untied if(r-l >= (1<<14)) //firstprivate (array, l, r) //shared(array)// 
                merge_sort_parallel(array, l, m);
                #pragma omp task shared(array) untied if(r-l >= (1<<14)) //firstprivate (array, l, r) //shared(array)//
                merge_sort_parallel(array, m + 1, r);
                #pragma omp taskyield
            }
            #pragma omp taskwait
            merge(array, l, m, r);
        } else {
            insertion_sort(array, l, r + 1);
        }
    } else {
        return;
    }
}

void merge_sort_parallel_wrapper(unsigned int arr[], unsigned int l, unsigned int r) {
    #pragma omp parallel num_threads(100)
    {
        #pragma omp single
        merge_sort_parallel(arr, l, r);
    }
}

int main() {
    // Generate random array for sorting
    unsigned int num_elements = 100000;
    unsigned int random_array1[num_elements];
    unsigned int random_array2[num_elements];

    generate_random_array(random_array1, num_elements);
    generate_random_array(random_array2, num_elements);

    auto arr_size = sizeof(random_array1) / sizeof(random_array1[0]);

    // print short_array(random_array, arr_size);

    // int split[] = {1,3,5,7,2,4,6,8};
    // int split[] = {2,4,6,8,1,3,5,7};
    // merge(split, 0, 3, 7);
    // print short_array(split, 8);

    // insertion_sort(random_array, 0, arr_size);

    //merge_sort_serial(random_array, 0, arr_size);

    auto start1 = high_resolution_clock::now();
    merge_sort_serial(random_array1, 0, arr_size);
    auto stop1 = high_resolution_clock::now();

    auto duration1 = duration_cast<microseconds>(stop1 - start1);

    auto start2 = high_resolution_clock::now();
    merge_sort_parallel_wrapper(random_array2, 0, arr_size);
    auto stop2 = high_resolution_clock::now();

    auto duration2 = duration_cast<microseconds>(stop2 - start2);

    // print short_array(random_array1, arr_size);
    // print short_array(random_array2, arr_size);

    cout << "Time taken by serial sort: "
         << duration1.count() << " microseconds" << endl;
    cout << "Time taken by parallel sort: "
         << duration2.count() << " microseconds" << endl;
    cout <<"Overall speedup: "
         << (float)duration1.count() / (float)duration2.count() << "x" << endl;
}
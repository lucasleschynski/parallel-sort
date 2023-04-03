#include <omp.h>
#include <random>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

void print_array(int array[], int length) {
    printf("[");
    for(int i = 0;i<length;i++) {
        i < length-1 ? printf("%d, ", array[i]) : printf("%d", array[i]);
    }
    printf("]\n");
}

void generate_random_array(int list[], int length) {
    int i;
    srand((unsigned int)time(NULL));
    for(i = 0; i < length; i++) {
        list[i] = rand() % (length*2);
    }
}

void insertion_sort(int array[], int l, int r) {
    int i, key, j;
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
void merge( int array[], int l, int m, int r) {
    int length = r - l + 1;
    int a = l;
    int b = m+1;
    int *fullcopy = new int[length];
    int i = 0;
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

void merge_sort_serial( int array[], int l, int r) {
    if(l < r) {
        if(l-r >= 43) {
            int m = l + ((r-l) / 2);
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

void merge_sort_parallel( int array[], int l, int r) {
    if(l < r) {
        if(l-r >= 43) {
            int m = l + ((r-l) / 2);
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

void merge_sort_parallel_wrapper( int arr[], int l, int r) {
    #pragma omp parallel num_threads(100)
    {
        #pragma omp single
        merge_sort_parallel(arr, l, r);
    }
}

int main() {
    // Generate random array for sorting
    int num_elements = 100000;
    int random_array1[num_elements];
    int random_array2[num_elements];

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
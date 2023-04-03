#include <omp.h>
#include <random>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

void print_array(vector<unsigned int> &v, unsigned int length) {
    printf("[");
    for(int i = 0;i<length;i++) {
        i < length-1 ? printf("%d, ", v[i]) : printf("%d", v[i]);
    }
    printf("]\n");
}

void generate_random_array(vector<unsigned int> &list, unsigned int length) {
    unsigned int i;
    srand((unsigned int)time(NULL));
    for(i = 0; i < length; i++) {
        list[i] = rand() % (length*2);
    }
}

void insertion_sort(vector<unsigned int> &v, unsigned int l, unsigned int r) {
    unsigned int i, key, j;
    for (i = l+1; i < r+1; i++) {
        key = v[i];
        j = i - 1;
        while (j >= 0 && v[j] > key) {
            v[j + 1] = v[j];
            j = j - 1;
        }
        v[j + 1] = key;
    }
}

// Merges two subarrays of array[] -> array[l..m] and array[m+1..r]
void merge(vector<unsigned int> &v, unsigned int l, unsigned int m, unsigned int r) {
    unsigned int length = r - l + 1;
    unsigned int a = l;
    unsigned int b = m+1;
    vector<unsigned int> fullcopy = vector<unsigned int>(length);
    unsigned int i = 0;
    while(a <= m && b <= r) {
        if(v[a] < v[b]) {
            fullcopy[i] = v[a];
            a++;
        } else {
            fullcopy[i] = v[b];
            b++;
        }
        i++;
    }
    bool left_merged = a > m ? true : false;
    if(left_merged) {
        while(b <= r) {
            fullcopy[i] = v[b];
            b++;
            i++;
        }
    } else {
        while(a <= m) {
            fullcopy[i] = v[a];
            a++;
            i++;
        }
    }
    for(int i = 0; i < length; i++) {
        v[l+i] = fullcopy[i];
    }
}

void merge_sort_serial(vector<unsigned int> &v, unsigned int l, unsigned int r) {
    if(l < r) {
        if(l-r >= 43) {
            unsigned int m = l + ((r-l) / 2);
            merge_sort_serial(v, l, m);
            merge_sort_serial(v, m + 1, r);
            merge(v, l, m, r);
        } else {
            insertion_sort(v, l, r + 1);
        }
    } else {
        return;
    }
}

void merge_sort_parallel(vector<unsigned int> &v, unsigned int l, unsigned int r) {
    if(l < r) {
        if(l-r >= 43) {
            unsigned int m = l + ((r-l) / 2);
            #pragma omp taskgroup 
            {
                #pragma omp task shared(v) untied if(r-l >= (1<<14)) //firstprivate (array, l, r) //shared(array)// 
                merge_sort_parallel(v, l, m);
                #pragma omp task shared(v) untied if(r-l >= (1<<14)) //firstprivate (array, l, r) //shared(array)//
                merge_sort_parallel(v, m + 1, r);
                #pragma omp taskyield
            }
            #pragma omp taskwait
            merge(v, l, m, r);
            // #pragma omp taskwait
        } else {
            insertion_sort(v, l, r + 1);
        }
    } else {
        return;
    }
}

void merge_sort_parallel_wrapper(vector<unsigned int> &v, unsigned int l, unsigned int r) {
    #pragma omp parallel num_threads(100)
    {
        #pragma omp single
        merge_sort_parallel(v, l, r);
    }
}

int main() {
    // Generate random array for sorting
    unsigned int num_elements = 1000000;
    vector<unsigned int> random_v1 = vector<unsigned int>(num_elements);
    vector<unsigned int> random_v2 = vector<unsigned int>(num_elements);

    generate_random_array(random_v1, num_elements);
    generate_random_array(random_v2, num_elements);

    auto v_size = random_v1.size();


    // print_array(random_array, arr_size);

    // int split[] = {1,3,5,7,2,4,6,8};
    // int split[] = {2,4,6,8,1,3,5,7};
    // merge(split, 0, 3, 7);
    // print_array(split, 8);

    // insertion_sort(random_array, 0, arr_size);

    //merge_sort_serial(random_array, 0, arr_size);

    auto start1 = high_resolution_clock::now();
    merge_sort_serial(random_v1, 0, v_size);
    auto stop1 = high_resolution_clock::now();

    auto duration1 = duration_cast<microseconds>(stop1 - start1);

    auto start2 = high_resolution_clock::now();
    merge_sort_parallel_wrapper(random_v2, 0, v_size);
    auto stop2 = high_resolution_clock::now();

    auto duration2 = duration_cast<microseconds>(stop2 - start2);

    // print_array(random_array1, arr_size);
    // print_array(random_array2, arr_size);

    cout << "Time taken by serial sort: "
         << duration1.count() << " microseconds" << endl;
    cout << "Time taken by parallel sort: "
         << duration2.count() << " microseconds" << endl;
    cout <<"Overall speedup: "
         << (float)duration1.count() / (float)duration2.count() << "x" << endl;
}
# parallel-sort

Exploring parallelism for mergesort with OpenMP

## Motivations

I have recently been learning about high-performance computing and have developed quite an interest in it. After reading about parallelism and distributed computing, I decided to play around with it a bit myself.

Prior to doing this, I had some experience in C, but not C++. Obviously, C++ is very applicable to these things, so I have also used this project as an exercise to learn the basics of C++.

## The "What" and "How"

This section describes the iterations the code has gone through with details on what prompted each iteration and the things learned at each step.

### Learning about parallelism

Prior to doing any coding, I did a fair bit of research on parallelism online. Once I felt that I had a good understanding of it, I decided to try my hand at implementing it.

### Sequential merge sort

First, I wrote the regular sequential version of merge sort. This served as a bit of a warmup and refresher on C-style programming.

### Basic parallelism

After adding the basic merge sort algorithm, I made a new parallel version. To do this, I used OpenMP.
In merge sort, the splitting of the array into smaller bits is parallelizeable. This is done below in the `merge_sort_parallel` function:

```cpp
// m = middle, l = left, r = right
int m = l + ((r-l) / 2);

#pragma omp taskgroup
{
    #pragma omp task shared(array) untied if(r-l >= (1<<14))
    merge_sort_parallel(array, l, m);
    #pragma omp task shared(array) untied if(r-l >= (1<<14))
    merge_sort_parallel(array, m + 1, r);
    #pragma omp taskyield
}
#pragma omp taskwait
merge(array, l, m, r);
```

The divide-and-conquer part of the algorithm is divided into two tasks which can run in parallel (using `#pragma omp task`).

The `shared(array)` clause allows multiple threads to access the array at the same time. When using this clause, OpenMP doesn't place any restrictions on shared data access and does nothing to prevent race conditions. However, because the two processes defined in each recursive instance of the algorithm divide the array into two explicit halves, race conditions are not a concern.

The `if(r-l >= (1<<X))` clause specifies that the task should only be parallelized if the size of the array is greater than $2^X$. Paralellizing parallelizable processes (little tongue twister there) is great, but there is quite a lot of overhead associated with doing so. If the task is too small, parallelizing ends up being more expensive than just performing it sequentially due to these overheads. I found that the sweet spot for $X$ for merge sort is about 10.

The `taskyield` statement allows the current threads (the few lines above it) to be suspended in favor of a different one. This doesn't make much of a difference here because the two tasks are essentially identical on average, but I put it there to declare this explicitly. The same goes for the `taskwait` statement, which specifies that the process above it must complete before executing. However, this is redundant in our case because the program won't reach the line below it until the recursive merges finish.

### Tweaking and Testing

After getting the parallelism working properly, I started testing different things and playing around with the code. This is where things got interesting.

Firstly, I realized that using the standard `signed int` does **not** work well for implementing an efficient merge sort. Here is why I think this is:

In each recursive call of the merge sort function, we calculate the middle index of the array by taking the left and right subarray indices and finding the midpoint (`m = l + ((r-l) / 2)`). This requires standard integer division **by 2**.

Because the standard `int` is signed, it has an MSB of 1, whereas the `unsigned int` has no important MSB. Because of this, the `unsigned int` can perform division by powers of 2 much more easily (which is what we need), as a simple right shift can be used to do so. The overhead from doing the more complex division need for a signed `int` adds up quickly and slows the algorithm down a lot.

###

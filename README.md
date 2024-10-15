Contains two implementations of a parallel merge sort algorithm designed to sort a list of integers using threads:

	1.	sort_list_threads.c: This program uses POSIX threads (pthreads) to implement a parallel merge sort. Each thread is responsible for sorting a sublist of the array, and the merge process is done iteratively across multiple levels. Synchronization is handled using barriers to ensure that threads merge their sublists in parallel at each level of the algorithm.
	2.	sort_list_openmp.c: This version implements the parallel merge sort using OpenMP. OpenMP provides an easier interface for parallelizing code. The sorting is done by multiple threads, and sublists are merged iteratively, similar to the pthreads implementation.

Compilation:

To compile the files, you can use the following commands:

For the pthreads implementation:

gcc -o sort_list_threads sort_list_threads.c -lpthread

For the OpenMP implementation:

gcc -fopenmp -o sort_list_openmp sort_list_openmp.c

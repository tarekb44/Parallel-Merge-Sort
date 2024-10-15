Contains two implementations of a parallel merge sort algorithm designed to sort a list of integers using threads:

	1.	sort_list_threads.c: This program uses POSIX threads (pthreads) to implement a parallel merge sort. Each thread is responsible for sorting a sublist of the array, and the merge process is done iteratively across multiple levels. Synchronization is handled using barriers to ensure that threads merge their sublists in parallel at each level of the algorithm.
	2.	sort_list_openmp.c: This version implements the parallel merge sort using OpenMP. OpenMP provides an easier interface for parallelizing code. The sorting is done by multiple threads, and sublists are merged iteratively, similar to the pthreads implementation.

Files:

	1.	sort_list_threads.c: This C program implements parallel merge sort using pthreads. It dynamically spawns threads based on user input (q, log base 2 of the number of threads), sorts sublists using quicksort, and then merges these sublists using a parallel merge process.
	2.	sort_list_openmp.c: This is an alternative implementation using OpenMP. It leverages OpenMP’s parallel for construct to distribute the sorting and merging tasks among threads. This version also supports dynamic thread allocation based on user input.

Compilation:

To compile the files, you can use the following commands:

For the pthreads implementation:

gcc -o sort_list_threads sort_list_threads.c -lpthread

For the OpenMP implementation:

gcc -fopenmp -o sort_list_openmp sort_list_openmp.c

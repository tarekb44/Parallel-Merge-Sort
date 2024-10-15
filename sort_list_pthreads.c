//
// Sorts a list using multiple threads
//

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#define MAX_THREADS     65536
#define MAX_LIST_SIZE   100000000

#define DEBUG 0

// Thread variables
//
// VS: ... declare thread variables, mutexes, condition varables, etc.,
// VS: ... as needed for this assignment 
//

// Global variables
int num_threads;		// Number of threads to create - user input 
int list_size;			// List size
int *list;			// List of values
int *work;			// Work array
int *list_orig;			// Original list of values, used for error checking

typedef struct thread_arg{
    int thread_id;
    int q;
    int num_threads;
    int list_size;
    int *list;
    int *work;
    pthread_barrier_t *barrier;
}thread_arg_t;


// Print list - for debugging
void print_list(int *list, int list_size) {
    int i;
    for (i = 0; i < list_size; i++) {
        printf("[%d] \t %16d\n", i, list[i]); 
    }
    printf("--------------------------------------------------------------------\n"); 
}

// Comparison routine for qsort (stdlib.h) which is used to 
// a thread's sub-list at the start of the algorithm
int compare_int(const void *a0, const void *b0) {
    int a = *(int *)a0;
    int b = *(int *)b0;
    if (a < b) {
        return -1;
    } else if (a > b) {
        return 1;
    } else {
        return 0;
    }
}

// Return index of first element larger than or equal to v in sorted list
// ... return last if all elements are smaller than v
// ... elements in list[first], list[first+1], ... list[last-1]
//
//   int idx = first; while ((v > list[idx]) && (idx < last)) idx++;
//
int binary_search_lt(int v, int *list, int first, int last) {
   
    // Linear search code
    // int idx = first; while ((v > list[idx]) && (idx < last)) idx++; return idx;

    int left = first; 
    int right = last-1; 

    if (list[left] >= v) return left;
    if (list[right] < v) return right+1;
    int mid = (left+right)/2; 
    while (mid > left) {
        if (list[mid] < v) {
	    left = mid; 
	} else {
	    right = mid;
	}
	mid = (left+right)/2;
    }
    return right;
}
// Return index of first element larger than v in sorted list
// ... return last if all elements are smaller than or equal to v
// ... elements in list[first], list[first+1], ... list[last-1]
//
//   int idx = first; while ((v >= list[idx]) && (idx < last)) idx++;
//
int binary_search_le(int v, int *list, int first, int last) {

    // Linear search code
    // int idx = first; while ((v >= list[idx]) && (idx < last)) idx++; return idx;
 
    int left = first; 
    int right = last-1; 

    if (list[left] > v) return left; 
    if (list[right] <= v) return right+1;
    int mid = (left+right)/2; 
    while (mid > left) {
        if (list[mid] <= v) {
	    left = mid; 
	} else {
	    right = mid;
	}
	mid = (left+right)/2;
    }
    return right;
}

//per thread
void *sublist_sort(void *arg_ptr) {
    thread_arg_t *arg = (thread_arg_t *)arg_ptr;
    int thread_id = arg->thread_id;
    int q = arg->q;
    int num_threads_local = arg->num_threads;
    int list_size_local = arg->list_size;
    int *list_local = arg->list;
    int *work_local = arg->work;
    pthread_barrier_t *barrier = arg->barrier;

    int np = list_size_local / num_threads_local;
    int start = thread_id * np;
    int end = (thread_id == num_threads_local - 1) ? list_size_local : start + np;

    qsort(&list_local[start], end - start, sizeof(int), compare_int);

    pthread_barrier_wait(barrier);

    for (int level = 0; level < q; level++) {
        int step = 1 << level;
        if (thread_id % (step * 2) == 0) {
            int partner_id = thread_id + step;
            if (partner_id < num_threads_local) {
                int merge_start = start;
                int merge_mid = merge_start + np * step;
                int merge_end = merge_mid + np * step;
                if (merge_end > list_size_local) merge_end = list_size_local;

                int A_start = merge_start;
                int A_end = merge_mid;
                int B_start = merge_mid;
                int B_end = merge_end;
                int C_start = merge_start;

                int i = A_start;
                int j = B_start;
                int k = C_start;

                while (i < A_end && j < B_end)
                {
                    if (list_local[i] <= list_local[j]) 
                    {
                        int idx_i = binary_search_le(list_local[j], list_local, i, A_end);
                        int num_elements_i = idx_i - i;

                        if (num_elements_i > 0) {
                            memcpy(&work_local[k], &list_local[i], num_elements_i * sizeof(int));
                            k += num_elements_i;
                            i = idx_i;
                        }
                    }
                    else 
                    {
                        int idx_j = binary_search_lt(list_local[i], list_local, j, B_end);
                        int num_elements_j = idx_j - j;
                        if (num_elements_j > 0)
                        {
                            memcpy(&work_local[k], &list_local[j], num_elements_j * sizeof(int));
                            k += num_elements_j;
                            j = idx_j;
                        }
                    }
                }
                if (i < A_end) 
                {
                    memcpy(&work_local[k], &list_local[i], (A_end - i) * sizeof(int));
                    k += (A_end - i);
                }

                if (j < B_end) {
                    memcpy(&work_local[k], &list_local[j], (B_end - j) * sizeof(int));
                    k += (B_end - j);
                }

                for (i = merge_start; i < merge_end; i++)
                    list_local[i] = work_local[i];
            }
        }
        pthread_barrier_wait(barrier);
    }
    pthread_exit(NULL);
}

// Parallel sort function
void sort_list(int q) {
    int num_threads_local = 1 << q;

    pthread_t threads[MAX_THREADS];

    pthread_barrier_t barrier;
    thread_arg_t args[MAX_THREADS];

    if (pthread_barrier_init(&barrier, NULL, num_threads_local))
    {
        fprintf(stderr, "Could not create a barrier\n");
        exit(0);
    }

    if (DEBUG) {
        printf("initial sorted:\n");
        print_list(list, list_size);
    }

    for (int i = 0; i < num_threads_local; i++)
    {
        args[i].thread_id = i;
        args[i].q = q;
        args[i].num_threads = num_threads_local;
        args[i].list_size = list_size;
        args[i].list = list;
        args[i].work = work;
        args[i].barrier = &barrier;

        if (pthread_create(&threads[i], NULL, sublist_sort, (void *)&args[i]))
        {
            fprintf(stderr, "pthread_create failed");
            exit(0);
        }
    }

    for (int i = 0; i < num_threads_local; i++)
        pthread_join(threads[i], NULL);
    if (DEBUG) {
        printf("final sorted list:\n");
        print_list(list, list_size);
    }
    pthread_barrier_destroy(&barrier);
}

// Main function
int main(int argc, char *argv[]) {
    struct timespec start, stop, stop_qsort;
    double total_time, total_time_qsort;
    int k, q, j, error; 

    // Read input, validate
    if (argc != 3) {
        printf("Need two integers as input \n"); 
        printf("Use: <executable_name> <log_2(list_size)> <log_2(num_threads)>\n"); 
        exit(0);
    }
    k = atoi(argv[1]);
    q = atoi(argv[2]);
    list_size = (1 << k);
    num_threads = (1 << q);

    if (list_size > MAX_LIST_SIZE || num_threads > MAX_THREADS || num_threads > list_size) {
        fprintf(stderr, "Invalid input values. Exiting.\n");
        exit(0);
    }

    // Allocate list, list_orig, and work
    list = (int *) malloc(list_size * sizeof(int));
    list_orig = (int *) malloc(list_size * sizeof(int));
    work = (int *) malloc(list_size * sizeof(int));

    if (!list || !list_orig || !work) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(0);
    }

//
// VS: ... May need to initialize mutexes, condition variables, 
// VS: ... and their attributes
//

    // Initialize list of random integers; list will be sorted by 
    // multi-threaded parallel merge sort
    // Copy list to list_orig; list_orig will be sorted by qsort and used
    // to check correctness of multi-threaded parallel merge sort
    srand48(0); 	// seed the random number generator
    for (j = 0; j < list_size; j++) {
	list[j] = (int) lrand48();
	list_orig[j] = list[j];
    }
    // duplicate first value at last location to test for repeated values
    list[list_size-1] = list[0]; list_orig[list_size-1] = list_orig[0];

    // Create threads; each thread executes find_minimum
    clock_gettime(CLOCK_REALTIME, &start);

//
// VS: ... may need to initialize mutexes, condition variables, and their attributes
//

// Serial merge sort 
// VS: ... replace this call with multi-threaded parallel routine for merge sort
// VS: ... need to create threads and execute thread routine that implements 
// VS: ... parallel merge sort

    sort_list(q);

    // Compute time taken
    clock_gettime(CLOCK_REALTIME, &stop);
    total_time = (stop.tv_sec-start.tv_sec)
	+0.000000001*(stop.tv_nsec-start.tv_nsec);

    // Check answer
    qsort(list_orig, list_size, sizeof(int), compare_int);
    clock_gettime(CLOCK_REALTIME, &stop_qsort);
    total_time_qsort = (stop_qsort.tv_sec-stop.tv_sec)
	+0.000000001*(stop_qsort.tv_nsec-stop.tv_nsec);

    error = 0; 
    for (j = 1; j < list_size; j++) {
	if (list[j] != list_orig[j]) error = 1; 
    }

    if (error != 0) {
	printf("Houston, we have a problem!\n"); 
    }

    // Print time taken
    printf("List Size = %d, Threads = %d, error = %d, time (sec) = %8.4f, qsort_time = %8.4f\n", 
	    list_size, num_threads, error, total_time, total_time_qsort);

// VS: ... destroy mutex, condition variables, etc.

    free(list); free(work); free(list_orig); 

}


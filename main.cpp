#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define MIN(a,b)  ((a)<(b)?(a):(b))

int main (int argc, char *argv[])
{
    int    count = 0;        /* Local prime count */
    double elapsed_time; /* Parallel execution time */
    int    first;        /* Index of first multiple */
    int    global_count = 0; /* Global prime count */
    int    high_value;   /* Highest value on this proc */
    int    i;
    int    id;           /* Process ID number */
    int    index;        /* Index of current prime */
    int    low_value;    /* Lowest value on this proc */
    char  *marked;       /* Portion of 2,...,'n' */
    int    n;            /* Sieving from 2, ..., 'n' */
    int    p;            /* Number of processes */
    int    proc0_size;   /* Size of proc 0's subarray */
    int    prime;        /* Current prime */
    int    size;         /* Elements in 'marked' */

    /*!!! n>p^2 !!! */
    MPI_Init (&argc, &argv);

    /* Start the timer */

    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();

    if (argc != 2) {
        if (!id) printf ("Command line: %s <m>\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }

    n = atoi(argv[1]);

    /* Figure out this process's share of the array, as
       well as the integers represented by the first and
       last array elements */

    low_value = 3 + id*(n-2)/p;
    if (!(low_value%2)) ++low_value; //if low_value is not odd (is even).
    high_value = 2 + (id+1)*(n-2)/p;
    if (!(high_value%2)) --high_value; //if high_value is not odd (is even).
    size = (high_value - low_value)/2 + 1;

    /* Bail out if all the primes used for sieving are
       not all held by process 0 */

    proc0_size = (n-1)/p;

    if ((2 + proc0_size) < (int) sqrt((double) n)) {
        if (!id) printf ("Too many processes\n");
        MPI_Finalize();
        exit (1);
    }

    /* Allocate this process's share of the array. */

    int num_sqrtn = floor(sqrt(n));
    if (!(num_sqrtn%2)) num_sqrtn--;
    num_sqrtn = (num_sqrtn-3)/2 + 1;
    char* calp = (char *) malloc(num_sqrtn*sizeof(char));

    marked = (char *) malloc ((size)*sizeof(char));
    //16
    int block_size=2<<16; //separate marked by every block_size elements

    if (marked == NULL) {
        printf ("Cannot allocate enough memory\n");
        MPI_Finalize();
        exit (1);
    }

    for (i = 0; i < size; i++) marked[i] = 0;
    for (i = 0; i < num_sqrtn; i++) calp[i] = 0;
    index = 0;
    prime = 3;

    // calculate all the primes need to sieve by.
    do {
        for (int i = 2 * index * index + 6 * index + 3; i < num_sqrtn; i += prime) {
            calp[i] = 1;
        }
        while (calp[++index]);
        prime = index * 2 + 3;
        global_count++;
    } while (prime*prime<=n);

    int* primes = (int *) malloc(global_count*sizeof(int));
    int j=0;
    for (int i=0; i<num_sqrtn; i++) {
        if (!calp[i]) primes[j++]= i * 2 + 3;
    }


    int num_blocks=size/block_size+1;
    for (int block_num=0;block_num<num_blocks;block_num++){
        int block_low_index = block_num * block_size;
        int block_low_value = low_value + block_low_index * 2;
        for (int j=0;j<global_count;j++){
            prime = primes[j];

            if (prime * prime > block_low_value)
                first = (prime * prime - block_low_value)/2;
            else {
                first = prime - (block_low_value % prime);
                if (first == prime) first=0;
                else if (first%2) first = (first + prime)/2;
                else first = first/2;
            }
            first+= block_low_index;

            int end = block_low_index + block_size > size?size:block_low_index + block_size;

            for (int i = first;i< end;i+=prime){
                marked[i] = 1;
            }
        }
    }



    for (i = 0; i < size; i++)
        if (!marked[i]) {
            count++;
        }
    MPI_Reduce (&count, &global_count, 1, MPI_INT, MPI_SUM,
                0, MPI_COMM_WORLD);

    /* Stop the timer */

    elapsed_time += MPI_Wtime();

    free(marked);
    free(calp);
    free(primes);

    /* Print the results */

    if (!id) {
        ++global_count;
        printf ("There are %d primes less than or equal to %d\n",
                global_count, n);
        printf ("SIEVE (%d) %10.6f\n", p, elapsed_time);
    }
    MPI_Finalize ();
    return 0;
}
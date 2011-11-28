/* sort.c 
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

/* size of physical memory; with code, we'll run out of space!*/
#define ARRAYSIZE 1024

int A[ARRAYSIZE];
char buffer[80];

int
main()
{
    int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < ARRAYSIZE; i++)		
        A[i] = ARRAYSIZE - i;

    /* then sort! */
    for (i = 0; i < ARRAYSIZE - 1; i++) {
        for (j = 0; j < (ARRAYSIZE - 1 - i); j++) {
	   if (A[j] > A[j + 1]) {	/* out of order -> need to swap ! */
	      tmp = A[j];
	      A[j] = A[j + 1];
	      A[j + 1] = tmp;
    	   }
        }
    }

    itoa (A[0], buffer);
    strcat (buffer, " <---\n");
    Write (1, buffer, strlen (buffer));

    Exit(A[0]);		
    /* According to Berkeley:  "and then we're done -- should be 0!" */
    /* 
     * If only they realized that 0 isn't even one of the elements involved.
     * This should return 1 (the smallest element in the array).
     */
}

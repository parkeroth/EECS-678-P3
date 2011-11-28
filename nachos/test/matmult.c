/* matmult.c 
 *    Test program to do matrix multiplication on large arrays.
 *
 *    Intended to stress virtual memory system.
 *
 *    Ideally, we could read the matrices off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"
#include "stdlib.h"

#define Dim 	20	/* sum total of the arrays doesn't fit in 
			 * physical memory 
			 */

int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];

char buffer[60];

int
main()
{
    int i, j, k;
#if defined(FORK2) || defined(FORK4) || defined(FORK8)
    int pid;
#endif

#if defined(FORK2) || defined(FORK4) || defined(FORK8)
    pid = Fork();
#endif
#if defined(FORK4) || defined(FORK8)
    pid = Fork();
#endif
#if defined(FORK8)
    pid = Fork(); 
#endif

    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
	for (j = 0; j < Dim; j++) {
	     A[i][j] = i;
	     B[i][j] = j;
	     C[i][j] = 0;
	}

    for (i = 0; i < Dim; i++)		/* then multiply them together */
	for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
		 C[i][j] += A[i][k] * B[k][j];

    itoa (C[Dim-1][Dim-1], buffer);
    strcat (buffer, " <---[");
    itoa (GetPID(), buffer + strlen (buffer));
    strcat (buffer, "]\n");
    Write (1, buffer, strlen (buffer));

#if defined(FORK2) || defined(FORK4) || defined(FORK8)
    /* 
     * Rely on the fact that Wait will check if we have children and return
     * immediately if not 
     */
    Wait (0);
    Wait (0);
    Wait (0);
#endif

    Exit(C[Dim-1][Dim-1]);		/* and then we're done */
    /* not reached */
    return 0;
}

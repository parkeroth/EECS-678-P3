/* count.c 
 *
 * This is a variation on the nice.c exercise of multiple
 * threads and varied static priorities. However, in this case, 
 * we add a compute bound loop, to ad a period of CPU use between
 * I/O operations. 
 */

#include "syscall.h"
#include "stdlib.h"

#define OUTPUT_LOOP_LIMIT  500
#define COMPUTE_LOOP_LIMIT 100

/*
 * Set this up to experiment with different nice values.  Negative
 * values can cause unexpected execution orders. Figuring out why
 * is an excellent exercise for the student.
 */

#define A_NICE 0
#define B_NICE 5
#define C_NICE 10
#define D_NICE 15

int
main()
{
    int a,b,c,d;
    int a2,b2,c2,d2;
    int a3,b3,c3,d3;
    OpenFileId output = ConsoleOutput;
    int pid;

    Write (output, "Starting...\n", 12);

    /* First Fork */
    pid = Fork(); 

    if (pid != 0) {
      /* 
       * Parent process of first fork 
       * Now we fork a second time.
       */
        pid = Fork(); 
        if (pid != 0) {
	   /*
	    * Original Parent process makes it here. Now we adjust its
	    * static priority by calling the NICE system call, and
	    * then looping to output a series of identifying
	    * characters. Each output character is delayed by
	    * the compute loop.
	    */
	    Nice (A_NICE);
            for (a = 0; a < OUTPUT_LOOP_LIMIT; a++) {
	        a3=0;
                for (a2 = 0; a2 < COMPUTE_LOOP_LIMIT; a2++) {
                    a3++;
                }
                Write (output, "A", 1);
            }
	    Wait (0);
        } else {
	   /*
	    * The second child of the original Parent process makes it
	    * here. Now we adjust its static priority by calling the
	    * NICE system call, and then looping to output a series of
	    * identifying characters. Each output character is delayed by
	    * the compute loop.
	    */
	    Nice (B_NICE);
            for (b = 0; b < OUTPUT_LOOP_LIMIT; b++) {
	        b3=0;
                for (b2 = 0; b2 < COMPUTE_LOOP_LIMIT; b2++) {
                    b3++;
                }
                Write (output, "B", 1);
            }
	    Exit (0);
        }
    } else {
      /*
       * the child of the first fork comes here and then 
       * forks a child of its own.
       */
        pid = Fork(); 
        if (pid != 0) {
	   /*
	    * The first child of the original Parent process makes it
	    * here. Now we adjust its static priority by calling the
	    * NICE system call, and then looping to output a series of
	    * identifying characters. Each output character is delayed by
	    * the compute loop.
	    */
	    Nice (C_NICE);
            for (c = 0; c < OUTPUT_LOOP_LIMIT; c++) {
	        c3=0;
                for (c2 = 0; c2 < COMPUTE_LOOP_LIMIT; c2++) {
                    c3++;
                }
                Write (output, "C", 1);
            }
            Wait (0);
	    Exit (0);
        } else {
	   /*
	    * The grandchild of the original Parent process makes it
	    * here. Now we adjust its static priority by calling the
	    * NICE system call, and then looping to output a series of
	    * identifying characters. Each output character is delayed by
	    * the compute loop.
	    */
	    Nice (D_NICE);
            for (d = 0; d < OUTPUT_LOOP_LIMIT; d++) {
	        d3=0;
                for (d2 = 0; d2 < COMPUTE_LOOP_LIMIT; d2++) {
                    d3++;
                }
                Write (output, "D", 1);
            }
	    Exit (0);
        }
    }

    Wait (0);
    Write (output, "\nFinished\n", 10);
    Exit(0);		/* and then we're done */
}



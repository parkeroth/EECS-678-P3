/* nice_console.c 
 *   Test program to run several threads with different priorities.
 *   Each thread uses the NICE system call to adjust its static 
 *   priority by various amounts, to influence execution order. 
 *
 *   This version of nice uses the console to present the execution
 *   order of the threads.
 */

#include "syscall.h"
#include "stdlib.h"

#define OUTPUT_LOOP_LIMIT 500

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
    OpenFileId output = ConsoleOutput;
    int pid;

    Write (output, "Starting...\n", 12);

    /* Divorce the test processes from the main process. The main
     * thread will create the others and then simply wait for them to
     * finish. This step is taken to ensure that the each of the four
     * threads is treated as similarly as possible.
     */

    /* Zero-th fork */
    pid = Fork();

    if (pid == 0)
      {
	/* for the child (the 4 threads we're testing) */

	/* First Test Process Fork */
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
	     * characters.
	     */
	    Nice (A_NICE);
            for (a = 0; a < OUTPUT_LOOP_LIMIT; a++) {
	      Write (output, "A", 1);
            }
	    Exit (0);
	  } else {
	    /*
	     * The second child of the original Parent process makes it
	     * here. Now we adjust its static priority by calling the
	     * NICE system call, and then looping to output a series of
	     * identifying characters.
	     */
	    Nice (B_NICE);
            for (b = 0; b < OUTPUT_LOOP_LIMIT; b++) {
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
	     * identifying characters.
	     */
	    Nice (C_NICE);
            for (c = 0; c < OUTPUT_LOOP_LIMIT; c++) {
	      Write (output, "C", 1);
            }
	    Exit (0);
	  } else {
	    /*
	     * The grandchild of the original Parent process makes it
	     * here. Now we adjust its static priority by calling the
	     * NICE system call, and then looping to output a series of
	     * identifying characters.
	     */
	    Nice (D_NICE);
            for (d = 0; d < OUTPUT_LOOP_LIMIT; d++) {
	      Write (output, "D", 1);
            }
	    Exit (0);
	  }
	}
      }

    Wait (0);
    Write (output, "\nFinished\n", 10);
    Exit(0);		/* and then we're done */
}

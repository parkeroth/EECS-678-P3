/* nice_free.c 
 *   Test program to run several threads with different priorities.
 *   Each thread uses the NICE system call to adjust its static 
 *   priority by various amounts, to influence execution order. 
 *
 *   This version of nice does not use the console, it uses a
 *   debugging system call, Echo, instead.
 */

#include "syscall.h"
#include "stdlib.h"

#define OUTPUT_LOOP_LIMIT 500

/*
 * Set this up to experiment with different nice values.  Negative
 * values can cause unexpected execution orders. Figuring out WHY
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
    int pid;

    Echo ("Starting...\n", 12);

    /* Divorce the test processes from the main process. The main
     * thread will create the others and then simply wait for them to
     * finish. This step is taken to ensure that the each of the four
     * threads is treated as similarly as possible.
     */


    /* Commence the forking */
    pid = Fork();

    if (pid == 0)
      {
	/* 
	 * Child #1: This will fork the other test processes as well
	 * as becoming T-A 
	 */

	/* Child #1 Forks a Grandchild #1 */
	pid = Fork(); 

	if (pid != 0) {
	  /* 
	   * Child #1  
	   * Now we fork a second time.
	   * Grandchild #2 is created
	   */
	  pid = Fork(); 
	  if (pid != 0) {
	    /*
	     * Child #1: The first Child process makes it here. Now we
	     * adjust its static priority by calling the NICE system
	     * call, and then looping to output a series of
	     * identifying characters.
	     */
		NameThread("T-A");
	    Nice (A_NICE);
            for (a = 0; a < OUTPUT_LOOP_LIMIT; a++) {
	      Echo ("A", 1);
            }
	    Exit (0);
	  } else {
	    /*
	     * Grandchild #2
	     * Now we adjust its static priority by calling the
	     * NICE system call, and then looping to output a series of
	     * identifying characters.
	     */
		NameThread("T-B");
	    Nice (B_NICE);
            for (b = 0; b < OUTPUT_LOOP_LIMIT; b++) {
	      Echo ("B", 1);
            }
	    Exit (0);
	  }
	} else {
	  /*
	   * Grandchild #1 forks GreatGrandchild #1
	   */
	  pid = Fork(); 
	  if (pid != 0) {
	    /*
	     * GrandChild #1
	     * Now we adjust its static priority by calling the
	     * NICE system call, and then looping to output a series of
	     * identifying characters.
	     */
		NameThread("T-C");
	    Nice (C_NICE);
            for (c = 0; c < OUTPUT_LOOP_LIMIT; c++) {
	      Echo ("C", 1);
            }
	    Exit (0);
	  } else {
	    /*
	     * GreatGrandchild #1 
	     * Now we adjust its static priority by calling the
	     * NICE system call, and then looping to output a series of
	     * identifying characters.
	     */
		NameThread("T-D");
	    Nice (D_NICE);
            for (d = 0; d < OUTPUT_LOOP_LIMIT; d++) {
	      Echo ("D", 1);
            }
	    Exit (0);
	  }
	}
      }
    /*Original Thread*/
    Wait (0);
    Echo ("\nFinished\n", 10);
    Exit(0);		/* and then we're done */
}

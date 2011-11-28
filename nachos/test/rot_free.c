/* rot_free.c 
 *   Test program to run four threads with equivalent priorities.
 *
 *   This application is to test the rule of n: these programs should
 *   show some inertia when they reach the CPU, they should not take
 *   one quantum turns.
 *
 *   This version of nice does not use the console, it uses a
 *   debugging system call, Echo, instead.
 */

#include "syscall.h"
#include "stdlib.h"

#define OUTPUT_LOOP_LIMIT 500

/*
 * Set this up to experiment with different nice values.  Negative
 * values can cause unexpected execution orders. Figuring out why
 * is an excellent exercise for the student.
 */

int
main()
{
    int a,b,c,d;
    int pid;

    Echo ("Starting...\n", 12);

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
	    * characters.
	    */
            for (a = 0; a < OUTPUT_LOOP_LIMIT; a++) {
                Echo ("A", 1);
            }
	    Wait (0);
        } else {
	   /*
	    * The second child of the original Parent process makes it
	    * here. Now we adjust its static priority by calling the
	    * NICE system call, and then looping to output a series of
	    * identifying characters.
	    */
            for (b = 0; b < OUTPUT_LOOP_LIMIT; b++) {
                Echo ("B", 1);
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
            for (c = 0; c < OUTPUT_LOOP_LIMIT; c++) {
                Echo ("C", 1);
            }
            Wait (0);
	    Exit (0);
        } else {
	   /*
	    * The grandchild of the original Parent process makes it
	    * here. Now we adjust its static priority by calling the
	    * NICE system call, and then looping to output a series of
	    * identifying characters.
	    */
            for (d = 0; d < OUTPUT_LOOP_LIMIT; d++) {
                Echo ("D", 1);
            }
	    Exit (0);
        }
    }

    Wait (0);
    Echo ("\nFinished\n", 10);
    Exit(0);		/* and then we're done */
}

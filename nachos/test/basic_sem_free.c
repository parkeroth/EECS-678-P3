/* sem_free.c 
 *   Test program to synchronize several threads using semaphores.
 *
 *   NOTE: This program does not use the console, it uses a special
 *   non-blocking system call, Echo, instead.
 */

#include "syscall.h"
#include "stdlib.h"

#define OUTER_LOOP_LIMIT 10
#define INNER_LOOP_LIMIT 50

#define SEM_ONE   "BB-S1"
#define SEM_TWO   "BB-S2"
#define SEM_THREE "BB-S3"
#define SEM_FOUR  "BB-S4"

void process_A()
{
  int in, out, sem_hdl;

  for (out = 0; out < OUTER_LOOP_LIMIT; out++) {
	for (in = 0; in < INNER_LOOP_LIMIT; in++) {
	  Echo ("A", 1);
	}
  }

  Exit (0);
}

void process_B()
{
  int in, out, sem_hdl;

  for (out = 0; out < OUTER_LOOP_LIMIT; out++) {
	for (in = 0; in < INNER_LOOP_LIMIT; in++) {
	  Echo ("B", 1);
	}
  }

  Exit (0);
}

void process_C()
{
  int in, out, sem_hdl;

  for (out = 0; out < OUTER_LOOP_LIMIT; out++) {
	for (in = 0; in < INNER_LOOP_LIMIT; in++) {
	  Echo ("C", 1);
	}
  }

  Exit (0);
}

void process_D()
{
  int in, out, sem_hdl;

  for (out = 0; out < OUTER_LOOP_LIMIT; out++) {
	for (in = 0; in < INNER_LOOP_LIMIT; in++) {
	  Echo ("D", 1);
	}
  }

  Exit (0);
}

int
main()
{
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
         * Original Parent process makes it here. Grab the AB-Semaphore and
         * begin looping to output a series of identifying characters.
	     */
		process_A();
	  } else {
	    /*
         * The second child of the original Parent process makes it here. Grab
         * the AB-Semaphore and begin looping to output a series of identifying
         * characters.
	     */
		process_B();
	  }
	} else {
	  /*
	   * the child of the first fork comes here and then 
	   * forks a child of its own.
	   */
	  pid = Fork(); 
	  if (pid != 0) {
	    /*
         * The first child of the original Parent process makes it here. Grab
         * the CD-Semaphore and begin looping to output a series of identifying
         * characters.
	     */
		process_C();
	  } else {
	    /*
         * The grandchild of the original Parent process makes it here. Grab
         * the CD-Semaphore and begin looping to output a series of identifying
         * characters.
	     */
		process_D();
	  }
	}
      }

    Wait (0);
    Echo ("\nFinished\n", 10);
    Exit(0);		/* and then we're done */
}

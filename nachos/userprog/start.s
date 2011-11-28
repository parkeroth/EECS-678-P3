/* FIXME: start.s should not be in the test directory.  Move it somewhere
 * appropriate.  It is also a very obscure place to put system call stubs. */
/* Start.s 
 *	Assembly language assist for user programs running on top of Nachos.
 *
 *	Since we don't want to pull in the entire C library, we define
 *	what we need for a user program here, namely Start and the system
 *	calls.
 */

#define IN_ASM
#include "syscall.h"

        .text   
        .align  2

/* -------------------------------------------------------------
 * _start
 *	Initialize running a C program, by calling "main". 
 *
 * 	NOTE: This has to be first, so that it gets loaded at location 0.
 *	The Nachos kernel always starts a program by jumping to location 0.
 * -------------------------------------------------------------
 */
	.globl	_start
	.ent	_start
_start:
	la	$gp,_gp
	jal	main
	move	$4,$0		
	jal	Exit	 /* if we return from main, exit(0) */
	.end	_start
//---------------------------------

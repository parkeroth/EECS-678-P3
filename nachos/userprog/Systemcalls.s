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
 * System call stubs:
 *	Assembly language assist to make system calls to the Nachos kernel.
 *	There is one stub per system call, that places the code for the
 *	system call into register r2, and leaves the arguments to the
 *	system call alone (in other words, arg1 is in r4, arg2 is 
 *	in r5, arg3 is in r6, arg4 is in r7)
 *
 * 	The return value is in r2. This follows the standard C calling
 * 	convention on the MIPS.
 * -------------------------------------------------------------
 */

	.globl Halt
	.ent	Halt
Halt:
	addiu $2,$0,SC_Halt
	syscall
	bgez	$2,$HaltPos
	subu	$3,$0,$2
	sw	$3,errno
	li	$2,-1
	j	$HaltDone
$HaltPos:
	sw	$0,errno
$HaltDone:	j	$31
	.end Halt

	.globl Exit
	.ent	Exit
Exit:
	addiu $2,$0,SC_Exit
	syscall
	bgez	$2,$ExitPos
	subu	$3,$0,$2
	sw	$3,errno
	li	$2,-1
	j	$ExitDone
$ExitPos:
	sw	$0,errno
$ExitDone:	j	$31
	.end Exit

	.globl Exec
	.ent	Exec
Exec:
	addiu $2,$0,SC_Exec
	syscall
	/* If the return value was >= 0, there was no error */
	bgez	$2,$ExecPos
	/* If there was an error, errno = - return value */
	subu	$3,$0,$2
	sw	$3,errno
	/*  and, the return value becomes -1 */
	li	$2,-1
	j	$ExecDone
	/* $ExecPos means there was no error, so errno = 0 */
$ExecPos:
	sw	$0,errno
	/* At this point all processing is done and we can return */
$ExecDone:
	j	$31
	.end Exec

	.globl Wait
	.ent	Wait
Wait:
	addiu $2,$0,SC_Wait
	syscall
	bgez	$2,$WaitPos
	subu	$3,$0,$2
	sw	$3,errno
	li	$2,-1
	j	$WaitDone
$WaitPos:
	sw	$0,errno
$WaitDone:
	j	$31
	.end Wait

	.globl Create
	.ent	Create
Create:
	addiu $2,$0,SC_Create
	syscall
	bgez	$2,$CreatePos
	subu	$3,$0,$2
	sw	$3,errno
	li	$2,-1
	j	$CreateDone
$CreatePos:
	sw	$0,errno
$CreateDone:
	j	$31
	.end Create

	.globl Open
	.ent	Open
Open:
	addiu $2,$0,SC_Open
	syscall
	bgez	$2,$OpenPos
	subu	$3,$0,$2
	sw	$3,errno
	li	$2,-1
	j	$OpenDone
$OpenPos:
	sw	$0,errno
$OpenDone:
	j	$31
	.end Open

	.globl Read
	.ent	Read
Read:
	addiu $2,$0,SC_Read
	syscall
	bgez	$2,$ReadPos
	subu	$3,$0,$2
	sw	$3,errno
	li	$2,-1
	j	$ReadDone
$ReadPos:
	sw	$0,errno
$ReadDone:
	j	$31
	.end Read

	.globl Write
	.ent	Write
Write:
	addiu $2,$0,SC_Write
	syscall
	bgez	$2,$WritePos
	subu	$3,$0,$2
	sw	$3,errno
	li	$2,-1
	j	$WriteDone
$WritePos:
	sw	$0,errno
$WriteDone:
	j	$31
	.end Write

	.globl Close
	.ent	Close
Close:
	addiu $2,$0,SC_Close
	syscall
	bgez	$2,$ClosePos
	subu	$3,$0,$2
	sw	$3,errno
	li	$2,-1
	j	$CloseDone
$ClosePos:
	sw	$0,errno
$CloseDone:
	j	$31
	.end Close

	.globl Fork
	.ent	Fork
Fork:
	addiu $2,$0,SC_Fork
	syscall
	bgez	$2,$ForkPos
	subu	$3,$0,$2
	sw	$3,errno
	li	$2,-1
	j	$ForkDone
$ForkPos:
	sw	$0,errno
$ForkDone:
	j	$31
	.end Fork

	.globl Yield
	.ent	Yield
Yield:
	addiu $2,$0,SC_Yield
	syscall
	bgez	$2,$YieldPos
	subu	$3,$0,$2
	sw	$3,errno
	li	$2,-1
	j	$YieldDone
$YieldPos:
	sw	$0,errno
$YieldDone:
	j	$31
	.end Yield

        .globl GetPID
        .ent    GetPID
GetPID:
        addiu $2,$0,SC_GetPID
        syscall
        bgez    $2,$GetPIDPos
        subu    $3,$0,$2
        sw      $3,errno
        li      $2,-1
        j       $GetPIDDone
$GetPIDPos:
        sw      $0,errno
$GetPIDDone:
        j       $31
        .end GetPID

        .globl GetPPID
        .ent    GetPPID
GetPPID:
        addiu $2,$0,SC_GetPPID
        syscall
        bgez    $2,$GetPPIDPos
        subu    $3,$0,$2
        sw      $3,errno
        li      $2,-1
        j       $GetPPIDDone
$GetPPIDPos:
        sw      $0,errno
$GetPPIDDone:
        j       $31
        .end GetPPID

        .globl Nice
        .ent   Nice
Nice:
        addiu $2,$0,SC_Nice
        syscall
        bgez    $2,$NicePos
        subu    $3,$0,$2
        sw      $3,errno
        li      $2,-1
        j       $NiceDone
$NicePos:
        sw      $0,errno
$NiceDone:
        j       $31
        .end Nice


	.globl NameThread
	.ent	NameThread
NameThread:
	addiu $2,$0,SC_NameThread
	syscall
	bgez	$2,$NameThreadPos
	subu	$3,$0,$2
	sw	$3,errno
	li	$2,-1
	j	$NameThreadDone
$NameThreadPos:
	sw	$0,errno
$NameThreadDone:
	j	$31
	.end NameThread


/* ---------------------------------- */
        .globl NachosUserEvent
        .ent   NachosUserEvent
NachosUserEvent:
        addiu $2,$0,SC_NachosUserEvent
        syscall
        bgez    $2,$NachosUserEventPos
        subu    $3,$0,$2
        sw      $3,errno
        li      $2,-1
        j       $NachosUserEventDone
$NachosUserEventPos:
        sw      $0,errno
$NachosUserEventDone:
        j       $31
        .end NachosUserEvent

/* ---------------------------------- */


	.globl	Echo
	.ent	Echo
Echo:
	addiu $2,$0,SC_Echo
	syscall
	bgez	$2,$EchoPos
	subu	$3,$0,$2
	sw	$3,errno
	li	$2,-1
	j	$EchoDone
$EchoPos:
	sw	$0,errno
$EchoDone:
	j	$31
	.end Echo
	
/* dummy function to keep gcc happy */
        .globl  __main
        .ent    __main
__main:
        j       $31
        .end    __main

	.comm	errno,4

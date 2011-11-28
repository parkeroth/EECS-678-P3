// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

#include "systemcall.h"
#include "nachos_dsui.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    int fault_delta;

    if (which == SyscallException) {
	do_system_call(type);
    } else if (which == PageFaultException) {
      int virtaddr = machine->ReadRegister(BadVAddrReg);

      stats->numPageFaults++;
      if (currentThread) {
          currentThread->procStats->numPageFaults++;
      }
      // There is one page fault histogram for the system.

      // stats->userTicks is the system level variable tracking user
      // ticks of *ALL* threads, therefore if the system load is
      // multi-threaded this would need to be generalized to track
      // histograms per thread
      fault_delta = stats->userTicks - stats->ticksAtLastPageFault;
      DSTRM_EVENT(EXCEPTION, UserTicksSinceLastPageFault, fault_delta);
      stats->ticksAtLastPageFault = stats->userTicks;

      memory->pagein( virtaddr / PageSize, currentThread->space );
    } else {
#ifdef REMOTE_USER_PROGRAM_DEBUGGING
	if (remoteDebugger) remoteDebugger->GDBCatchException(which);
	else {
#endif
	printf("Unexpected user mode exception (%d %d), process=%s, PID=%d\n",
	       which, type, currentThread->GetName (), 
	       currentThread->Get_Id ());
	currentThread->Finish ();
	ASSERT(false);
#ifdef REMOTE_USER_PROGRAM_DEBUGGING
	}
#endif
    }
}

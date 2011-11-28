// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (KernelSemaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// reset the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#pragma implementation "threads/synch.h"

#include "copyright.h"
#include "synch.h"
#include "system.h"
#include "thread.h"


//----------------------------------------------------------------------
// KernelSemaphore::~KernelSemaphore
// 	Deallocate semaphore, when no longer needed.  Wake up anything
//	still waiting on the semaphore.
//----------------------------------------------------------------------

KernelSemaphore::~KernelSemaphore()
{
  Thread *thread = static_cast<Thread *>(queue.Remove());

  while (thread != NULL)
    {
      scheduler->ReadyToRun(thread);
      thread = static_cast<Thread *>(queue.Remove());
    }
}

//----------------------------------------------------------------------
// KernelSemaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
KernelSemaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue.Append(currentThread);		// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// KernelSemaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that Interrupts
//	are disabled when it is called.
//----------------------------------------------------------------------

void
KernelSemaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = static_cast<Thread *>(queue.Remove());
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// Lock::~Lock
//     De-allocate lock, when no longer needed.  Wake up anything still
//     waiting on the lock.
//----------------------------------------------------------------------
Lock::~Lock()
{
  Thread *thread = static_cast<Thread *>(queue.Remove());

  while (thread != NULL)
    {
      scheduler->ReadyToRun(thread);
      thread = static_cast<Thread *>(queue.Remove());
    }
}

//----------------------------------------------------------------------
// Lock::Acquire
//     Wait until the lock is available.  This must be done atomically,
//     so we need to disable interrupts before checking availability.
//
//     Note that Thread::Sleep assumes that interrupts are disabled
//     when it is called.
//----------------------------------------------------------------------

void
Lock::Acquire()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);  // disable interrupts

    while (owner != NULL) {                    // lock not available
       queue.Append(currentThread);            // so go to sleep
       currentThread->Sleep();
     }
    owner = currentThread;                     // Make this thread the owner

    (void) interrupt->SetLevel(oldLevel);      // re-enable interrupts
}

//----------------------------------------------------------------------
// Lock::Release
//     Release the lock, waking up a waiter if necessary.  As with
//     Acquire(), this operation must be atomic, so we need to disable
//     interrupts.  Scheduler::ReadyToRun() assumes that threads
//     are disabled when it is called.
//----------------------------------------------------------------------

void
Lock::Release()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    owner = static_cast<Thread *>(queue.Remove());
    if (owner != NULL)
       scheduler->ReadyToRun(owner);
    (void) interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// Lock::isHeldByCurrentThread
//     Return true if the current thread is the holder of the lock,
//     false otherwise.
//----------------------------------------------------------------------

bool
Lock::isHeldByCurrentThread()
{
  return currentThread == owner;
}

//----------------------------------------------------------------------
// Condition::~Condition
//     De-allocate condition variable, when no longer needed.  Wake up
//     anything still waiting on the lock.
//----------------------------------------------------------------------

Condition::~Condition()
{
  Thread *thread = static_cast<Thread *>(queue.Remove());

  while (thread != NULL)
    {
      scheduler->ReadyToRun(thread);
      thread = static_cast<Thread *>(queue.Remove());
    }
}

//----------------------------------------------------------------------
// Condition::Wait
//     Wait on the condition variable.  The lock must be released while
//     the thread sleeps, then reacquire when it wakes up.
//
//     "conditionLock" is the lock associated with this c.v.
//----------------------------------------------------------------------

void Condition::Wait(Lock* conditionLock) {
    queue.Append(currentThread);               // Enter the queue
    conditionLock->Release();                  // Release the lock
    IntStatus oldLevel = interrupt->SetLevel(IntOff);  // disable interrupts
    currentThread->Sleep();
    (void) interrupt->SetLevel(oldLevel);
    conditionLock->Acquire();  // Acquire the lock again
}

//----------------------------------------------------------------------
// Condition::Signal
//     Signal the condition variable, in order to wake up one sleeping
//     thread, if any.
//
//     "conditionLock" is the lock associated with this c.v.
//----------------------------------------------------------------------

void Condition::Signal(Lock* conditionLock)
{
    conditionLock->Acquire();
    Thread *thread = static_cast<Thread *>(queue.Remove());
    if (thread != NULL)
       scheduler->ReadyToRun(thread);
    conditionLock->Release();
}

//----------------------------------------------------------------------
// Condition::Broadcast
//     Signal the condition variable, in order to wake up all sleeping
//     threads.
//
//     "conditionLock" is the lock associated with this c.v.
//----------------------------------------------------------------------

void Condition::Broadcast(Lock* conditionLock)
{
    Thread *thread;

    conditionLock->Acquire();
    for (thread = static_cast<Thread *>(queue.Remove());
	 thread != NULL;
	 thread = static_cast<Thread *>(queue.Remove()))
       scheduler->ReadyToRun(thread);
    conditionLock->Release();
}

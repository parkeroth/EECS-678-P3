// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1993-1994 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#pragma implementation "threads/scheduler.h"

#include "copyright.h"
#include "scheduler.h"
#include "system.h"
#include "nachos_dsui.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize a new scheduler
//----------------------------------------------------------------------
Scheduler::Scheduler (size_t quantum_in)
{
  threadid = 1;
  readyList.setComparator (ThreadOrder);
  quantum = quantum_in;
}


//----------------------------------------------------------------------
// Scheduler::ThreadOrder
// 	Order two threads by priority.
//----------------------------------------------------------------------

bool
ThreadOrder(void *thread1, void *thread2)
{
  // in order to use the arguments as threads, we must "down cast" the
  // types from void* to Thread*
  Thread *t1 = static_cast<Thread *>(thread1);
  Thread *t2 = static_cast<Thread *>(thread2);
  return t1->Get_Priority() < t2->Get_Priority();
}

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread *thread)
{
	DSTRM_EVENT(SCHED, READY_TO_RUN, thread->Get_Id());
    DEBUG((char *) DB_THREAD , (char *)"Putting thread %s on ready list, priority %d.\n",
	  thread->GetName(), thread->Get_Priority());

    // If a new thread is ready, then a scheduling decision is
    // needed. It is a new thread only if it isn't the current
    // thread. If it is the current thread, then a scheduling decision
    // is already happening; i.e. this function has been called from
    // Thread::Yield().
    if (thread != currentThread)
      {
	interrupt->setNeedResched();
      }

    // The thread is now in the ready state...
    thread->setStatus(READY);

    // ... so add it to the ready list
    readyList.Insert(thread);
}

// debugging function
void
debugShowReadyList( List& l )
{
  void debug_ancillary_tprint( size_t arg );

  if ( DebugIsEnabled( (char *)"show_readylist" ) ) {
    DEBUG( (char *)"show_readylist" , (char *)"\nREADYLIST is {" );

    l.Mapcar( debug_ancillary_tprint );

    DEBUG( (char *)"show_readylist" , (char *)"}\n" );
  }
}

void debug_ancillary_tprint( size_t arg )
{
  Thread *t = (Thread *) arg;
  DEBUG( (char *)"show_readylist" , (char *)" %s: dp=%d," , t->GetName() , t->Get_Priority() );
}


//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun ()
{
  // there must be a current thread and it must not be running
  ASSERT( NULL != currentThread );
  ASSERT( RUNNING != currentThread->getStatus() );

  Thread *lastThread, *nextThread;

  //
  // Check to see if all threads have run to completion
  //
  lastThread = (Thread *) allThreads.head->thread;

  if ( lastThread == (Thread *) allThreads.tail->thread &&
       lastThread->getStatus() == ZOMBIE )
    {
      //
      // If so, halt the system
      //
      interrupt->Halt();
    }
  
  // Choose the priority-wise best ready thread, the "candidate" for
  // replacing the current thread
  //
  nextThread = static_cast<Thread *>(readyList.Remove());

  return nextThread;
}


//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread)
{
    ASSERT( RUNNING != currentThread->getStatus() );

    Thread *oldThread = currentThread;



#ifdef USER_PROGRAM			// ignore until running user programs 
    if (currentThread->space != NULL) {	// if this thread is a user program,
        currentThread->SaveUserState(); // save the user's CPU registers
	currentThread->space->SaveState();
    }
#endif

    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    currentThread = nextThread;		    // switch to the next thread
    currentThread->setStatus(RUNNING);      // nextThread is now running

    DEBUG( (char *)DB_THREAD , (char *)"Switching from thread \"%s\" to thread \"%s\"\n",
	  oldThread->GetName(), nextThread->GetName());

    stats->numContextSwitches++;
    if (currentThread) {
        currentThread->procStats->numContextSwitches++;
        if (wasYieldOnReturn) {
	    currentThread->procStats->numInvolContextSwitches++;
	} else {
	    currentThread->procStats->numVolContextSwitches++;
	}
    }
    wasYieldOnReturn = false;


#ifndef USE_PTHREAD
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".
#endif /* USE_PTHREAD */
   if (oldThread != nextThread) {

	 DSTRM_EVENT_DATA(SCHED, SWITCH_FROM, oldThread->Get_Id(), 
			  sizeof(int), &(stats->totalTicks), "print_int"); 

       SWITCH(oldThread, nextThread);

       DSTRM_EVENT_DATA(SCHED, SWITCH_TO, currentThread->Get_Id(), 
			sizeof(int), &(stats->totalTicks), "print_int"); 
   }

    DEBUG( (char *)DB_THREAD , (char *)"Now in thread \"%s\"\n", currentThread->GetName());

    // If the old thread gave up the processor because it was finishing,
    // we need to delete its carcass.  Note we cannot delete the thread
    // before now (for example, in Thread::Finish()), because up to this
    // point, we were still running on the old thread's stack!
    if (threadToBeDestroyed != NULL) {
        delete threadToBeDestroyed;
	threadToBeDestroyed = NULL;
    }

#ifdef USER_PROGRAM
    if (currentThread->space != NULL) {		// if there is an address space
        currentThread->RestoreUserState();     // to restore, do it.
	currentThread->space->RestoreState();
    }
#endif
}


//----------------------------------------------------------------------
// Scheduler::PrintReadyList
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------

void
Scheduler::PrintReadyList()
{
    if ( !(DebugIsEnabled( (char *)"show_readylist" )) ) {
	// Set the tag
	DebugInit((char *)"show_readylist");

	debugShowReadyList( readyList );

	// Only want to print list when called, not every iteration or
    	// subsequent calls to debugShowReadyList that might be made
    	// elsewhere, therefore we remove the enabled tag 'show_readylist'.
    	DebugCleanUp();

    }
    else {	
    	debugShowReadyList( readyList );
    }
   
}


//----------------------------------------------------------------------
// Scheduler::addToList
//       This will add an element to the list of existing threads.
//----------------------------------------------------------------------

int
Scheduler::addToList (Thread* thread)
{
  struct nachos_thread *node = new nachos_thread;

  node->ID = threadid++;
  node->next = NULL;
  node->thread = (void *)thread;
  
  if (allThreads.tail) {
    allThreads.tail->next = node;
    allThreads.tail = node;
  } else {
    allThreads.tail = allThreads.head = node;
  }

  return node->ID;
}


//----------------------------------------------------------------------
// Scheduler::removeFromList
//           This takes an element off the list of existing threads.
//----------------------------------------------------------------------

void
Scheduler::removeFromList (Thread* thread)
{
  struct nachos_thread *tmp = allThreads.head, *prev = NULL;

  while (tmp) {
    if (tmp->thread == (void *)thread) {
      break;
    }
    prev = tmp;
    tmp = tmp->next;
  }
  if (tmp) {
    if (prev) {
      prev->next = tmp->next;
      if (prev->next == NULL) {
	allThreads.tail = prev;
      }
    } else {
      allThreads.head = tmp->next;
      if (allThreads.head->next == NULL) {
	allThreads.tail = tmp->next;
      }
    }
    delete tmp;
  }
}

#ifdef SMARTGDB
//----------------------------------------------------------------------
// Scheduler::GetThis
//           This takes an element off the list of existing threads.
//----------------------------------------------------------------------

Thread *
Scheduler::GetThis (Thread* thread)
{
  return static_cast<Thread *>(readyList.Remove (thread));
}
#endif


//----------------------------------------------------------------------
// Scheduler::SwitchTo()
//           This will switch to running a particular thread.  It it
//           mostly for use in debugging.  Note that if the thread is
//           null, or if it's not already ready, this function won't do
//           anything.
//----------------------------------------------------------------------
void Scheduler::Switch_To( Thread * newthread ) {
  
  // If no thread, return.
  if ( newthread == NULL ) {
    return;
  }

  // If thread is not ready to run, return.
  if ( newthread->getStatus() != READY ) {
    return;
  }

  //
  // Remove the thread from the ready list.
  // So we don't have it on there twice.
  //
  readyList.Delete (newthread);  

  //
  // Add it to the front of the readylist.
  //
  readyList.Prepend (newthread);
  
  currentThread->Yield();
}

size_t Scheduler::Quantum() const {
  return quantum;
}


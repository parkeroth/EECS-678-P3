// -*- C++ -*-
// scheduler.h 
//	Data structures for the thread dispatcher and scheduler.
//	Primarily, the list of threads that are ready to run.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SCHEDULER_H
#define SCHEDULER_H

#pragma interface "threads/scheduler.h"

#include "copyright.h"
#include "list.h"
#include "thread.h"
#include "system.h"
#include "nachos-gdb.h"

class Thread;

// The following class defines the scheduler/dispatcher abstraction -- 
// the data structures and operations needed to keep track of which 
// thread is running, and which threads are ready but not running.

bool ThreadOrder(void *, void *); // Order two threads by priority

class Scheduler {
public:
  Scheduler(size_t quantum_in);		// Initialize a scheduler

  void ReadyToRun(Thread* thread);	// Thread can be dispatched.
  Thread* FindNextToRun();		// Dequeue first thread on the ready 
					// list, if any, and return thread.
  void Run(Thread* nextThread);		// Cause nextThread to start running

  void PrintReadyList();		// Print contents of ready list

  int addToList(Thread * thread);	// Add a thread to the list
  void removeFromList(Thread * thread);	// Remove a thread from the list
#ifdef SMARTGDB
  Thread * GetThis( Thread * thread );
#endif
  void Switch_To( Thread * newthread );	// Switch to execute newthread.

  size_t Quantum() const;


private:
  size_t quantum;			// number of ticks between scheduling interrupts


  SortedList readyList;			// queue of threads that are ready to
					// run, but are not running

  int threadid;
};

#endif // SCHEDULER_H

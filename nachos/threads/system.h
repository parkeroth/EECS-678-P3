// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#pragma interface "threads/system.h"

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "memmgr.h"
#include "swapmgr.h"
#include "ifdef_symbols.h"




class Scheduler;
class Interrupt;
class MemoryManager;
class SwapManager;

enum PageReplPolicies {DUMB, FIFO, LRU, SECONDCHANCE};

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock
extern bool wasYieldOnReturn;			// involuntary context switch
extern bool printProcStats;			// print process statistics
extern int wsDeltaSize;				// Delta for working set
						// calculations
extern PageReplPolicies pageReplPolicy;		// Page-replacement policy
extern char* mainProgramName;			// -x parameter nachos was launched with

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
extern MemoryManager *memory;
extern SwapManager *swap;
#ifdef REMOTE_USER_PROGRAM_DEBUGGING
#include "nachos-stub.h"
extern int GDBRemotePort;
extern GDBRemoteDebugger * remoteDebugger;     // GDB remote debugger for user programs
#endif
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H

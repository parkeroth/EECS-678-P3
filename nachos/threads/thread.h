// thread.h 
//	Data structures for managing threads.  A thread represents
//	sequential execution of code within a program.
//	So the state of a thread includes the program counter,
//	the processor registers, and the execution stack.
//	
// 	Note that because we allocate a fixed size stack for each
//	thread, it is possible to overflow the stack -- for instance,
//	by recursing to too deep a level.  The most common reason
//	for this occuring is allocating large data structures
//	on the stack.  For instance, this will cause problems:
//
//		void foo() { int buf[1000]; ...}
//
//	Instead, you should allocate all data structures dynamically:
//
//		void foo() { int *buf = new int[1000]; ...}
//
//
// 	Bad things happen if you overflow the stack, and in the worst 
//	case, the problem may not be caught explicitly.  Instead,
//	the only symptom may be bizarre segmentation faults.  (Of course,
//	other problems can cause seg faults, so that isn't a sure sign
//	that your thread stacks are too small.)
//	
//	One thing to try if you find yourself with seg faults is to
//	increase the size of thread stack -- ThreadStackSize.
//
//  	In this interface, forking a thread takes two steps.
//	We must first allocate a data structure for it: "t = new Thread".
//	Only then can we do the fork: "t->fork(f, arg)".
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef THREAD_H
#define THREAD_H

#pragma interface "threads/thread.h"

#include "copyright.h"
#include "utility.h"
#include "defs.h"


class AddrSpace;

#ifdef USER_PROGRAM
#include "machine.h"
#include "addrspace.h"
#include "fdt.h"
#include "list.h"
#include "synch.h"
#include "stats.h"
#endif

#ifdef USE_PTHREAD
#include <pthread.h>
#endif

// CPU register state to be saved on context switch.  
// The SPARC and MIPS only need 10 registers, but the Snake needs 18.
// The Alpha needs to save 10 64bit registers
// For simplicity, this is just the max over all architectures.
#define MachineStateSize 20


// Size of the thread's private execution stack.
// WATCH OUT IF THIS ISN'T BIG ENOUGH!!!!!
#define StackSize	(4 * 32768)	// in words


// Externally available function wrapper whose sole job is to call
// Thread class method Print from MapCar when the readylist is being
// operated on.
extern void ThreadPrint(size_t arg);	 




// The following class defines a "thread control block" -- which
// represents a single thread of execution.
//
//  Every thread has:
//     an execution stack for activation records ("stackTop" and "stack")
//     space to save CPU registers while not running ("machineState")
//     a "status" (running/ready/blocked)
//    
//  Some threads also belong to a user address space; threads
//  that only run in the kernel have a NULL address space.

class Thread {
private:
  // NOTE: DO NOT CHANGE the order of these first two members.
  // THEY MUST be in this position for SWITCH to work.
  unsigned int* stackTop;		 // the current stack pointer
#ifdef HOST_ALPHA
  u_long machineState[MachineStateSize]; // all registers except for stackTop
#else
  unsigned int machineState[MachineStateSize];  // all registers except for stackTop
#endif
  
public:
  Thread();				// initialize a Thread 
  ~Thread(); 				// deallocate a Thread
  // NOTE -- thread being deleted
  // must not be running when delete 
  // is called

  // basic thread operations
  
  int Fork(VoidFunctionPtr func, size_t arg);   // Fork to create another thread
  void Yield();  				// Yield to another thread
#ifdef SMARTGDB
  void ForceYield( Thread * nextThread );
#endif                                         
  void Sleep();                                 // Put the thread to sleep and
  void Sleep (ThreadStatus newstatus);

  void Finish();  				// The thread is done executing
  void ReachedExit();		// done executing, but may be waiting
				// on children to finish
  bool HasReachedExit();	// query thread progress
  
  void CheckOverflow();   			// Check if thread has 
                                                // overflowed its stack

  void setStatus(ThreadStatus st);
  ThreadStatus getStatus () const;
  const char* GetName() const { return name; }
  void SetName (char *threadName) { 
    strncpy (name, threadName, MAXFILENAMELENGTH);
      *(name + MAXFILENAMELENGTH) = '\0';
  }


 void Print();  // Print the priority information of a thread OR just 
                // the ThreadID depending on the ifdef Flag (F_PRIORITY).
 int Get_Id(); // Returns the ThreadID

  // This is the implementation for the new priority driven system
  int Get_Priority(void);
  void Prioritize(int p);


  // This is a class data member that holds the value of the -R
  // command line parameter. It is used in KU's EECS 678 PA2-2
  // project.
  static double dpRetentionFactor;

  static bool wssContractionEnabled;



#ifdef USER_PROGRAM 
  // --------------------------------
  // Child Stuff
  // --------------------------------
  void Queue_Child( Thread * child );// Appends the child to the ChildList
  Thread* UnQueue_Child(); // Removes the child from the ChildList
  List ChildList; // List of Children for the currentThread
  Statistics *procStats; // Object to record performance metrics.
  
  Thread* Get_Parent_Ptr();  //Returns the Parent Pointer
  int Set_Parent_Ptr( Thread * parent ); // Sets the Parent pointer to parent.

  
  void Add_Child(); // Increment the Children variable
  void Remove_Child(); // Decrement the Children variable
  int Get_Num_Children(); // Returns the value of the variable Children
  int Children; // variable denoting the number of Children to the currentThread

  // -------------------------------
  // Exit Stuff
  // -------------------------------
  int Get_Exit_Val(); // Returns the value of exitval
  void Set_Exit_Val( int val ); // Sets the value of exitval to val
  KernelSemaphore* ChildExited;  
#endif

  bool thread_exit_status; // To signal to the parent that the child has finished execution

#ifdef USE_PTHREAD
  pthread_t realThread;
  pthread_mutex_t schedLock;
  pthread_cond_t schedCond;
  bool runNow;
#endif



  // book-keeping for when to reset the working set
  void resetWssRefreshCounter();
  bool incrWssRefreshCounter();
  void refreshWss();

 private:
  // NOTE: some of the private data for this class is listed above (for byte-hacking reasons)

  bool hasReachedExit;		// has the thread exhausted its code execution?

  unsigned int wssRefreshCounter;  // how many more quanta before this
				   // thread refreshes its working set
				   // size?

  // period for refreshing the working set size (units = quanta)
  static const unsigned int WSS_REFRESH_PERIOD = 10;


#ifdef USER_PROGRAM
    Thread* ParentPtr;
    int exitval;
#endif
    unsigned int* stack; 	// Bottom of the stack; NULL if this is the
				// main thread (if so, don't deallocate stack)
    ThreadStatus tstatus;	// ready, running or blocked
    char name [MAXFILENAMELENGTH + 1];
    int ThreadID;
#ifndef USE_PTHREAD
    void StackAllocate(VoidFunctionPtr func, size_t arg);
    				// Allocate a stack for thread.
				// Used internally by Fork()
#endif /* USE_PTHREAD */

    int Priority;		// The static priority of a process (thread)

#ifdef USER_PROGRAM
// A thread running a user program actually has *two* sets of CPU registers -- 
// one for its state while executing user code, one for its state 
// while executing kernel code.

    int userRegisters[NumTotalRegs];	// user-level CPU register state
    FDTEntry *FDTable [MAX_FD];         // File Descriptor Table entries

  public:
    void SaveUserState();		// save user-level register state
    void RestoreUserState();		// restore user-level register state
    void Write_Reg(int place, int value) { userRegisters[place] = value; }
    int Read_Reg(int place) { return userRegisters[place]; }
    AddrSpace *space;			// User code this thread is running.

    // File descriptor routines
    int find_next_available_fd (); // finds the next available File descriptor for
                                   // the thread
    void setFD (int fd, FDTEntry *file); // set a file descriptor value
    FDTEntry *getFD (int fd); // Return's a file descriptor value
#endif
};


// Magical machine-dependent routines, defined in switch.s

extern "C" {
// First frame on thread execution stack; 
//   	enable interrupts
//	call "func"
//	(when func returns, if ever) call ThreadFinish()
#ifdef USE_PTHREAD
  void *ThreadRoot (void *);
#else
  void ThreadRoot(); // After Fork, execution starts from this function
                     // so that returning from function or exiting is simple
                     // and straight forward, otherwise there would be no
                     // function to return to after execution of the function
                     // passed as a parameter to the Fork routine.
#endif

  // Stop running oldThread and start running newThread
  void SWITCH(Thread *oldThread, Thread *newThread);
}

#endif // THREAD_H

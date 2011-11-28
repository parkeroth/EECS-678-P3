// thread.cc 
//	Routines to manage threads.  There are four main operations:
//
//	Fork -- create a thread to run a procedure concurrently
//		with the caller (this is done in two steps -- first
//		allocate the Thread object, then call Fork on it)
//	Finish -- called when the forked procedure finishes, to clean up
//	Yield -- relinquish control over the CPU to another ready thread
//	Sleep -- relinquish control over the CPU, but thread is now blocked.
//		In other words, it will not run again, until explicitly 
//		put back on the ready queue.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#pragma implementation "threads/thread.h"

#include "copyright.h"
#include "thread.h"
#include "switch.h"
#include "synch.h"
#include "system.h"
#include "scheduler.h"
#include "nachos_dsui.h"

#define STACK_FENCEPOST 0xdeadbeef	// this is put at the top of the
					// execution stack, for detecting 
					// stack overflows

// This variable holds the value of the -R command line parameter. It
// is part of KU's EECS678 PA2-2 project.
double Thread::dpRetentionFactor = 0.0;

bool Thread::wssContractionEnabled = false;


#ifdef USE_PTHREAD
struct ForkArgs {
  VoidFunctionPtr func;
  size_t arg;
  Thread *thread;
};
#endif

//----------------------------------------------------------------------
// Thread::Print
//      Print thread information, usually for debugging purposes
//----------------------------------------------------------------------

void
Thread::Print() {
  printf("T-%d:S%d, ", ThreadID, Priority);	// print with static
}



//----------------------------------------------------------------------
// Thread::Get_Id
//      Return the thread's ID
//----------------------------------------------------------------------

int Thread::Get_Id() {
  return ThreadID;
}

//----------------------------------------------------------------------
// Thread::Get_Priority
//      Return the Process's (thread's) Priority
//----------------------------------------------------------------------

int Thread::Get_Priority() {
  return Priority;
}

//----------------------------------------------------------------------
// Thread::Prioritize
//      Sets the Process's (thread's) Priority
//      Modifies the Process's (thread's) Static Priority
//      Called by the NICE system Call
//      This enforces the assumption that NACHOS static priorities are 0-40
//----------------------------------------------------------------------

void Thread::Prioritize (int p) {
    Priority += p;
    if (Priority < 0)
       Priority = 0;
    if (Priority > 40)
       Priority = 40;
}





#ifdef USER_PROGRAM
//----------------------------------------------------------------------
// THE CHILDREN FUNCTIONS
//----------------------------------------------------------------------
int Thread::Get_Num_Children() {
  return Children;
}


void Thread::Add_Child() {
  Children++;
}


void Thread::Remove_Child() {
  if (Children < 1) {
    ASSERT (false);
  }
  Children--;
}


void Thread::Queue_Child(Thread * child) {
  ChildList.Append (child);
}


Thread* Thread::UnQueue_Child() {
  return static_cast<Thread *>(ChildList.Remove ());
}


Thread* Thread::Get_Parent_Ptr() {
  return ParentPtr;
}


int Thread::Set_Parent_Ptr( Thread * Parent ) {
  if ( ParentPtr != NULL ) {
    return -1;
  }
  ParentPtr = Parent;
  return 0;
}


int Thread::Get_Exit_Val() {
  return exitval;
}


void Thread::Set_Exit_Val( int val ) {
  exitval = val;
}
#endif


void Thread::resetWssRefreshCounter()
{
  wssRefreshCounter = 0;
}

bool Thread::incrWssRefreshCounter()
{
  if ( WSS_REFRESH_PERIOD == wssRefreshCounter )
    {
      resetWssRefreshCounter();
      return true;
    }
  else
    {
      ++ wssRefreshCounter;
      return false;
    }
}

void Thread::refreshWss()
{
    ASSERT( NULL != space ); // thread must have an address space

    // 1) determine how many pages were referenced in the recent history

    int refCount = 0;               //# pages being referenced
    unsigned char deltaMask;        //history mask for given delta size
    
    // 678 Set mask based on the value of Delta Size
    if(wsDeltaSize == 1)
    {
        deltaMask = 0x80;
    }
    else if(wsDeltaSize == 2)
    {
        deltaMask = 0xC0;
    }
    else if(wsDeltaSize == 4)
    {
        deltaMask = 0xF0;
    }
    else if(wsDeltaSize == 8)
    {
        deltaMask = 0xFF;
    }
    
    // 678 Count number of pages currently being referenced
    for(unsigned int i=0; i < space->getNumPages(); i++)
    {
        if(0x00 != (space->get_page_ptr(i)->history & deltaMask))
        {
            refCount++;
        }
    }

    // 678 Determine WS size and set
    //Minimum prescribed working set size, as given in project
    //outline
    if(refCount < 4)
    {
        currentThread->space->setWorkingSetSize(4);
    }
    //Maximum prescribed working set size, as given in project
    //outline
    else if(refCount > 32)
    {
        currentThread->space->setWorkingSetSize(32);
    }
    //Prescribed working set size somewhere in between
    else
    {
        currentThread->space->setWorkingSetSize(refCount);
    }

    if ( wssContractionEnabled )
    {   // --> the "cwss" command line parameter was specified, so the
	    // --> contract step is enabled
	    // 2) if the process owns too many pages, take them away; the "contract" step
        // 678
        
        int victim = 0;                            //victim for wss contraction
        while(space->TooManyFrames())
        {
            victim = memory->Choose_Victim(-1);    // Get victim
            memory->pageout(victim);               // Page out victim
        }
    }

    // 3) slide the history window
    // 678
    unsigned int newHistory;                //current page's new (shifted) history
    for(unsigned int page=0; page < space->getNumPages(); page++)
    {
        newHistory = space->get_page_ptr(page)->history >> 1;   //bitwise l2r shift
        space->get_page_ptr(page)->history = newHistory;        //set new history
    }
}

//----------------------------------------------------------------------
// Thread::Thread
// 	Initialize a thread control block, so that we can then call
//	Thread::Fork.
//----------------------------------------------------------------------

Thread::Thread()
{
    char tid_string[75];

    hasReachedExit = false;

#ifdef USER_PROGRAM
    ChildExited = new KernelSemaphore((char *)"ChildWait", 0);
    exitval = 0;
    Children = 0;
    ParentPtr = NULL;
#endif
    stackTop = NULL;
    stack = NULL;
    setStatus(JUST_CREATED);
    Priority = 20;
    wssRefreshCounter = 0;

    thread_exit_status = false;
    
    // Just after a thread is created, register it with the list in the
    // scheduler.
    ThreadID = scheduler->addToList (this);
    DSTRM_EVENT_DATA(THREAD, CLASS_CONSTRUCTOR, ThreadID, sizeof(int), &(stats->totalTicks), "print_int"); 

    // Build the thread name
    sprintf (tid_string, "T-%d", ThreadID);
    SetName (tid_string);

#ifdef USER_PROGRAM
    // Set up file descriptor table, special case stdin and stdout
    FDTable [0] = new FDTEntry;
    FDTable [0]->type = ConsoleFile;
    FDTable [1] = new FDTEntry;
    FDTable [1]->type = ConsoleFile;
    for (int i = 2; i < MAX_FD; i++) {
      FDTable [i] = NULL;
    }

    space = NULL;

    procStats = new Statistics();
#endif

#ifdef USE_PTHREAD
    runNow = false;
    pthread_mutex_init (&schedLock, NULL);
    pthread_cond_init (&schedCond, NULL);
#endif
}


//----------------------------------------------------------------------
// Thread::~Thread
// 	De-allocate a thread.
//
// 	NOTE: the current thread *cannot* delete itself directly,
//	since it is still running on the stack that we need to delete.
//
//      NOTE: if this is the main thread, we can't delete the stack
//      because we didn't allocate it -- we got it automatically
//      as part of starting up Nachos.
//----------------------------------------------------------------------

Thread::~Thread()
{

    DEBUG( (char *)DB_THREAD , (char *)"Deleting thread \"%s\"\n", name);

    ASSERT(this != currentThread);

    if (procStats) {
        delete procStats;
    }

#ifdef USER_PROGRAM
    for (int i = 0; i < MAX_FD; i++) { 
      if (FDTable [i]) {
	switch ( FDTable[i]->type ) {
	case ConsoleFile:
	break;
	case DiskFile:
	delete FDTable[i]->openfile;
	break;
	default:
	break;
	};	
	delete FDTable [i]; 
      }
    }
    delete space;
    delete ChildExited;
#endif

#ifndef USE_PTHREAD
    if (stack != NULL)
	DeallocBoundedArray((char *) stack, StackSize * sizeof(int));
#endif

    scheduler->removeFromList( this );
}

#ifdef USE_PTHREAD
void *ThreadRoot(void *arg_struct)
{
  struct ForkArgs *args = static_cast<struct ForkArgs *>(arg_struct);
  VoidFunctionPtr func = args->func;
  size_t arg = args->arg;
  Thread *t = args->thread;

  delete args;

  // Wait here until the scheduler says to run
  pthread_mutex_lock (&t->schedLock);
  if (!t->runNow)
    pthread_cond_wait (&t->schedCond, &t->schedLock);
  pthread_mutex_unlock (&t->schedLock);

  // Now run the appropriate functions
  DEBUG( (char *)DB_THREAD , (char *)"Starting thread \"%s\"\n", currentThread->GetName());
#ifdef USER_PROGRAM
  currentThread->RestoreUserState();
  if (currentThread->space != NULL) {
    currentThread->space->RestoreState();
  }
#endif
  interrupt->Enable();
  ((void (*)(size_t))func)(arg);
  t->Finish();

  return NULL;
}

void SWITCH(Thread *oldThread, Thread *newThread)
{
  if (oldThread != newThread)
    {
      pthread_mutex_lock (&oldThread->schedLock);

      // Unblock the thread we are switching to
      pthread_mutex_lock (&newThread->schedLock);

      newThread->runNow = true;
      pthread_cond_signal (&newThread->schedCond);
      pthread_mutex_unlock (&newThread->schedLock);

      // Is the thread we are switching from exiting?
      if (oldThread->getStatus() == ZOMBIE)
	pthread_exit (0);

      // Block the thread we are switching from
      pthread_cond_wait (&oldThread->schedCond, &oldThread->schedLock);
      pthread_mutex_unlock (&oldThread->schedLock);
    }
}
#endif

//----------------------------------------------------------------------
// Thread::Fork
// 	Invoke (*func)(arg), allowing caller and callee to execute 
//	concurrently.
//
//	NOTE: although our definition allows only a single integer argument
//	to be passed to the procedure, it is possible to pass multiple
//	arguments by making them fields of a structure, and passing a pointer
//	to the structure as "arg".
//
// 	Implemented as the following steps:
//		1. Allocate a stack
//		2. Initialize the stack so that a call to SWITCH will
//		cause it to run the procedure
// 	
//	"func" is the procedure to run concurrently.
//	"arg" is a single argument to be passed to the procedure.
//----------------------------------------------------------------------

int
Thread::Fork(VoidFunctionPtr func, size_t arg)
{
#ifdef USE_PTHREAD
    pthread_attr_t attr;
    ForkArgs *args = new ForkArgs;

    args->func   = func;
    args->arg    = arg;
    args->thread = this;
#endif

    DEBUG( (char *)DB_THREAD , (char *)"Forking thread \"%s\" with func = 0x%x, arg = %d\n",
	  name, (size_t) func, arg);
    
#ifdef USE_PTHREAD
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
    ASSERT (pthread_create (&realThread, &attr, ThreadRoot, args) == 0);
#else
    StackAllocate(func, arg);
#endif

    return ThreadID;
}    


//----------------------------------------------------------------------
// Thread::CheckOverflow
// 	Check a thread's stack to see if it has overrun the space
//	that has been allocated for it.  If we had a smarter compiler,
//	we wouldn't need to worry about this, but we don't.
//
// 	NOTE: Nachos will not catch all stack overflow conditions.
//	In other words, your program may still crash because of an overflow.
//
// 	If you get bizarre results (such as seg faults where there is no code)
// 	then you *may* need to increase the stack size.  You can avoid stack
// 	overflows by not putting large data structures on the stack.
// 	Don't do this: void foo() { int bigArray[10000]; ... }
//----------------------------------------------------------------------

void
Thread::CheckOverflow()
{
#ifndef USE_PTHREAD
    if (stack != NULL)
#ifdef HOST_SNAKE			// Stacks grow upward on the Snakes
	ASSERT(stack[StackSize - 1] == STACK_FENCEPOST);
#else
	ASSERT(*stack == STACK_FENCEPOST);
#endif
#endif
}

void Thread::setStatus(ThreadStatus st) {
  int status;

  // assign the status member
  tstatus = st;
  status = st;
  DSTRM_EVENT_DATA(THREAD, SET_STATUS, ThreadID, sizeof(int), &status, "print_int"); 
}

ThreadStatus Thread::getStatus () const { return tstatus; }


//----------------------------------------------------------------------
// Thread::Finish
// 	Called by ThreadRoot when a thread is done executing the 
//	forked procedure.
//
// 	NOTE: we don't immediately de-allocate the thread data structure 
//	or the execution stack, because we're still running in the thread 
//	and we're still on the stack!  Instead, we set "threadToBeDestroyed", 
//	so that Scheduler::Run() will call the destructor, once we're
//	running in the context of a different thread.
//
// 	NOTE: we disable interrupts, so that we don't get a time slice 
//	between setting threadToBeDestroyed, and going to sleep.
//----------------------------------------------------------------------

void
Thread::Finish ()
{
    (void) interrupt->SetLevel(IntOff);		
    ASSERT(this == currentThread);
    
    DEBUG((char *) DB_THREAD , (char *)"Finishing thread \"%s\"\n", GetName());
    DSTRM_EVENT_DATA(THREAD, FINISH, currentThread->Get_Id(), sizeof(int), &(stats->totalTicks), "print_int"); 
    
    threadToBeDestroyed = currentThread;
#ifdef USE_PTHREAD
    pthread_mutex_destroy (&schedLock);
    pthread_cond_destroy (&schedCond);
#endif

    Sleep(ZOMBIE);                                // invokes SWITCH
}

//----------------------------------------------------------------------
// Thread::ReachedExit
// 	Called by the system call handler for the Exit system call.
//
// 	NOTE: this is mostly a hook for any state-sensitive thread
//            analysis
//
//      Sets a flag in the PCB recording that the code of this thread
//      has called Exit, which indicates the end of the program's
//      liveness.
//----------------------------------------------------------------------

void
Thread::ReachedExit ()
{
  ASSERT(this == currentThread);
    
  DEBUG( (char *)DB_THREAD , (char *)"Reached exit; thread \"%s\"\n", GetName() );

  hasReachedExit = true;
  DSTRM_EVENT_DATA(THREAD, REACHED_EXIT, currentThread->Get_Id(), sizeof(int), &(stats->totalTicks), "print_int"); 

}

bool
Thread::HasReachedExit ()
{
  return hasReachedExit;
}


//----------------------------------------------------------------------
// Thread::Yield
// 	Relinquish the CPU if any other thread is ready to run.
//	If so, put the thread on the end of the ready list, so that
//	it will eventually be re-scheduled.
//
//	NOTE: returns immediately if no other thread on the ready queue.
//	Otherwise returns when the thread eventually works its way
//	to the front of the ready list and gets re-scheduled.
//
//	NOTE: we disable interrupts, so that looking at the thread
//	on the front of the ready list, and switching to it, can be done
//	atomically.  On return, we re-set the interrupt level to its
//	original state, in case we are called with interrupts disabled. 
//
// 	Similar to Thread::Sleep(), but a little different.
//----------------------------------------------------------------------

void
Thread::Yield ()
{
    Thread *nextThread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    
    ASSERT(this == currentThread);
    
    DEBUG( (char *)DB_THREAD , (char *)"Yielding thread \"%s\"\n", GetName());

    // Before making a scheduling decision, add the current thread to
    // the list of ready threads
    scheduler->ReadyToRun( this );
    nextThread = scheduler->FindNextToRun();

    // because at least the current thread is READY, there must be a
    // next thread
    ASSERT( NULL != nextThread );

    // run the next thread
    scheduler->Run(nextThread);

    (void) interrupt->SetLevel(oldLevel);
}

#ifdef SMARTGDB
//-------------------------------------------------------------
// Thread::ForceYield
//
// This function is called from SmartGDB to force a context
// switch.  What we need to do is get a thread in, then remove
// it from the readylist.  If we can't, we will return.
//--------------------------------------------------------------
void
Thread::ForceYield ( Thread * nextThread)
{

    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    
    ASSERT(this == currentThread);
    
    DEBUG( (char *)DB_THREAD , (char *)"Yielding thread \"%s\"\n", GetName());
  
  
    nextThread = scheduler->GetThis( nextThread );
    if (nextThread != NULL) {
	scheduler->ReadyToRun(this);
	scheduler->Run(nextThread);
    }
    (void) interrupt->SetLevel(oldLevel);
}
#endif
//----------------------------------------------------------------------
// Thread::Sleep
// 	Relinquish the CPU, because the current thread is blocked
//	waiting on a synchronization variable (KernelSemaphore, Lock, or
//	Condition). Eventually, some thread will wake this thread up, and put it
//	back on the ready queue, so that it can be re-scheduled.
//
//	NOTE: if there are no threads on the ready queue, that means
//	we have no thread to run.  "Interrupt::Idle" is called
//	to signify that we should idle the CPU until the next I/O interrupt
//	occurs (the only thing that could cause a thread to become
//	ready to run).
//
//	NOTE: we assume interrupts are already disabled, because it
//	is called from the synchronization routines which must
//	disable interrupts for atomicity.   We need interrupts off 
//	so that there can't be a time slice between pulling the first thread
//	off the ready list, and switching to it.
//----------------------------------------------------------------------
void
Thread::Sleep ()
{
    Thread *nextThread;
    
    ASSERT(this == currentThread);
    ASSERT(interrupt->getLevel() == IntOff);
    
    DEBUG( (char *)DB_THREAD , (char *)"Sleeping thread \"%s\"\n", GetName());
    setStatus(BLOCKED);

    while ((nextThread = scheduler->FindNextToRun()) == NULL)
	  interrupt->Idle();	// no one to run, wait for an interrupt
        
    scheduler->Run(nextThread); // returns when we've been signalled
}



//----------------------------------------------------------------------
// Thread::Sleep 
//       Puts the current process to sleep with the specified status
//----------------------------------------------------------------------
void
Thread::Sleep (ThreadStatus newstatus)
{
    Thread *nextThread;
    
    ASSERT(this == currentThread);
    ASSERT(newstatus != RUNNING);
    setStatus(newstatus);
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    while ((nextThread = scheduler->FindNextToRun()) == NULL)
	interrupt->Idle();	// no one to run, wait for an interrupt


    scheduler->Run(nextThread); // returns when we've been signalled
    (void) interrupt->SetLevel(oldLevel);
}


//----------------------------------------------------------------------
// ThreadFinish, InterruptEnable, ThreadPrint
//	Dummy functions because C++ does not allow a pointer to a member
//	function.  So in order to do this, we create a dummy C function
//	(which we can pass a pointer to), that then simply calls the 
//	member function.
//----------------------------------------------------------------------

#ifndef USE_PTHREAD
static void ThreadFinish()
{
  currentThread->Finish();
}

static void InterruptEnable()
{
  DEBUG( DB_THREAD , "Starting thread \"%s\"\n", currentThread->GetName());
#ifdef USER_PROGRAM
  currentThread->RestoreUserState();
  if (currentThread->space != NULL) {
    currentThread->space->RestoreState();
  }
#endif
  interrupt->Enable();
}
#endif

void ThreadPrint(size_t arg)
{
  Thread *t = (Thread *)arg;
  t->Print();
}


#ifndef USE_PTHREAD
//----------------------------------------------------------------------
// Thread::StackAllocate
//	Allocate and initialize an execution stack.  The stack is
//	initialized with an initial stack frame for ThreadRoot, which:
//		enables interrupts
//		calls (*func)(arg)
//		calls Thread::Finish
//
//	"func" is the procedure to be forked
//	"arg" is the parameter to be passed to the procedure
//----------------------------------------------------------------------

void
Thread::StackAllocate (VoidFunctionPtr func, size_t arg)
{
#ifndef USE_PTHREAD
    stack = (unsigned int *) AllocBoundedArray(StackSize * sizeof(int));

#ifdef HOST_SNAKE
    // HP stack works from low addresses to high addresses
    stackTop = stack + 16;	// HP requires 64-byte frame marker
    stack[StackSize - 1] = STACK_FENCEPOST;
#else
    // i386 & MIPS & SPARC stack works from high addresses to low addresses
#ifdef HOST_SPARC
    // SPARC stack must contains at least 1 activation record to start with.
    stackTop = stack + StackSize - 96;
#else  // HOST_MIPS  || HOST_i386 || HOST_ALPHA
#ifdef HOST_ALPHA
    stackTop = stack + StackSize - 8;
#else
    stackTop = stack + StackSize - 4;	// -4 to be on the safe side!
#endif
#ifdef HOST_i386
    // the 80386 passes the return address on the stack.  In order for
    // SWITCH() to go to ThreadRoot when we switch to this thread, the
    // return addres used in SWITCH() must be the starting address of
    // ThreadRoot.
    *(--stackTop) = (int)ThreadRoot;
#endif
#endif  // HOST_SPARC
    *stack = STACK_FENCEPOST;
#endif  // HOST_SNAKE
#endif // USE_PTHREAD
    
#ifdef HOST_ALPHA
    machineState[PCState] = (u_long) ThreadRoot;
    machineState[StartupPCState] = (u_long) InterruptEnable;
    machineState[InitialPCState] = (u_long) func;
    machineState[InitialArgState] = (u_long) arg;
    machineState[WhenDonePCState] = (u_long) ThreadFinish;
#else
    machineState[PCState] = (size_t) ThreadRoot;
    machineState[StartupPCState] = (size_t) InterruptEnable;
    machineState[InitialPCState] = (size_t) func;
    machineState[InitialArgState] = arg;
    machineState[WhenDonePCState] = (size_t) ThreadFinish;
#endif
}
#endif

#ifdef USER_PROGRAM
#include "machine.h"


//----------------------------------------------------------------------
// Thread::SaveUserState
//	Save the CPU state of a user program on a context switch.
//
//	Note that a user program thread has *two* sets of CPU registers -- 
//	one for its state while executing user code, one for its state 
//	while executing kernel code.  This routine saves the former.
//----------------------------------------------------------------------

void
Thread::SaveUserState()
{
    for (int i = 0; i < NumTotalRegs; i++)
	userRegisters[i] = machine->ReadRegister(i);
}


//----------------------------------------------------------------------
// Thread::RestoreUserState
//	Restore the CPU state of a user program on a context switch.
//
//	Note that a user program thread has *two* sets of CPU registers -- 
//	one for its state while executing user code, one for its state 
//	while executing kernel code.  This routine restores the former.
//----------------------------------------------------------------------

void
Thread::RestoreUserState()
{
    for (int i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, userRegisters[i]);
}


//----------------------------------------------------------------------
// Thread::find_next_available_fd
//     Finds the next available file descriptor for this thread
//----------------------------------------------------------------------
int Thread::find_next_available_fd () {
  for (int i=0; i < MAX_FD; i++) {
    if (FDTable[i] == NULL) {
      return i;
    }
  }
  return -1;
}


//----------------------------------------------------------------------
// Thread::setFD
//     Sets a file descriptor value
//----------------------------------------------------------------------
void Thread::setFD (int fd, FDTEntry *file) {
  if (fd < MAX_FD) {
    if (FDTable [fd]) {
      if ((FDTable [fd]->type == DiskFile) && (FDTable [fd]->openfile)) {
	delete FDTable [fd]->openfile;
      }
      delete FDTable [fd];
    }
    FDTable [fd] = file;
  }
}


//----------------------------------------------------------------------
// Thread::getFD
//     Returns a file descriptor's value
//----------------------------------------------------------------------
FDTEntry *Thread::getFD (int fd) {
  if (fd < MAX_FD) {
    return (FDTable [fd]);
  }
  else {
    return NULL;
  }
}

#endif

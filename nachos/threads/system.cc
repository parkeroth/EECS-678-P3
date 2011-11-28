// system.cc 
//	Nachos initialization and cleanup routines.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#pragma implementation "threads/system.h"

#include "copyright.h"
#include "system.h"
#include "memmgr.h"
#include "console.h"
#include "synch.h"
#include "nachos-gdb.h"
#include "nachos_dsui.h"



// This defines *all* of the global data structures used by Nachos.
// These are all initialized and de-allocated by this file.

Thread *currentThread;			// the thread we are running now
Thread *threadToBeDestroyed;  		// the thread that just finished
Scheduler *scheduler = NULL;		// the scheduler
Interrupt *interrupt;			// interrupt status
Statistics *stats;			// performance metrics
Timer *schedTimer;			// the hardware timer device,
					// for invoking context switches
char* mainProgramName;			// -x parameter nachos was launched with




// List of all existing threads
nachos_thread_list allThreads = {NULL, NULL};


#ifdef FILESYS_NEEDED
FileSystem  *fileSystem;
#endif

#ifdef FILESYS
SynchDisk   *synchDisk;
#endif

#ifdef USER_PROGRAM	// requires either FILESYS or FILESYS_STUB
#include "swapmgr.h"
Machine *machine;	// user program memory and registers
#ifdef REMOTE_USER_PROGRAM_DEBUGGING
int GDBRemotePort = 0;;
GDBRemoteDebugger * remoteDebugger;     // GDB remote debugger for user programs
#endif
MemoryManager *memory;
SwapManager *swap;
Console *console;
bool wasYieldOnReturn = false;
bool printProcStats = false;
int wsDeltaSize = 1;
PageReplPolicies pageReplPolicy = DUMB;

// The following two semaphores will control access to the console read and
// write operations. This is needed because a process may wish to initiate a 
// read/write of multiple bytes, but the console does things one byte at a 
// time. Thus, until a complete chunk of bytes has been read/written, we do
// not want any new read requests to begin. 
KernelSemaphore *consoleRead; 
KernelSemaphore *consoleWrite;

// These semaphores allow processes to sleep until a character is available 
// for reading, or until a write is complete. 
KernelSemaphore *consoleReadAvail;
KernelSemaphore *consoleWriteDone;

// These two are dummy function wrappers around the V() member functions of 
// the sempahores used for console I/O. 
static void ConsoleReadAvail(size_t arg) {
  arg = 0;                                             // Keep gcc happy
  consoleReadAvail->V();
}
static void ConsoleWriteDone(size_t arg) {
  arg = 0;                                             // Keep gcc happy
  consoleWriteDone->V();
}

#endif

#ifdef NETWORK
PostOffice *postOffice;
#endif


// External definition, to allow us to take a pointer to this function
extern void Cleanup();


//----------------------------------------------------------------------
// SchedTimerInterruptHandler
// 	Interrupt handler for the timer device.  The timer device is
//      set up to interrupt the CPU periodically (once every
//      SchedTimerTicks).  This routine is called each time there is a
//      timer interrupt, with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as 
//	if the interrupted thread called Yield at the point it is 
//	was interrupted.
//
//	"dummy" is because every interrupt handler takes one argument,
//		whether it needs it or not.
//----------------------------------------------------------------------
static void
SchedTimerInterruptHandler(size_t dummy)
{
  dummy = 0; // Keep gcc happy
  DSTRM_EVENT(SCHED, SCHED_TIMER, 0);

  if (interrupt->getStatus() != IdleMode)
    {
      // refresh the current thread's working set size, if its time
      if ( currentThread->incrWssRefreshCounter() )
	  currentThread->refreshWss();

      // possibly pre-empt the thread when the interrupt handler
      // returns (when the handler finishes and the thread is about to
      // resume processing)
      interrupt->YieldOnReturn();
    }
}



//----------------------------------------------------------------------
// Initialize
// 	Initialize Nachos global data structures.  Interpret command
//	line arguments in order to determine flags for the initialization.  
// 
//	"argc" is the number of command line arguments (including the name
//		of the command) -- ex: nachos -d "all" -> argc = 3 
//	"argv" is an array of strings, one for each command line argument
//		ex: nachos -d "all" -> argv = {"nachos", "-d", "all"}
//----------------------------------------------------------------------
void
Initialize(int argc, char **argv)
{
    int argCount;
    char* debugArgs = (char *)"";
    bool randomYield = false;
    bool noSwitch = false;

    // default (average) time between scheduling interrupts
    size_t schedTimerTicks = 110;
    // default number of inertial quanta


#ifdef USER_PROGRAM
    bool debugUserProg = false;	// single step user program
    
#endif
#ifdef FILESYS_NEEDED
    bool format = false;	// format disk
#endif
#ifdef NETWORK
    double rely = 1;		// network reliability
    int netname = 0;		// UNIX socket name
#endif
    
    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
	argCount = 1;
	if (!strcmp(*argv, "-d")) {
	  ASSERT(argc > 1);
	  debugArgs = *(argv + 1);
	  argCount = 2;
	} else if (!strcmp(*argv, "-rs")) {
	    ASSERT(argc > 1);
	    RandomInit(atoi(*(argv + 1)));	// initialize pseudo-random
						// number generator
	    randomYield = true;
	    argCount = 2;
	}
	else if (!strcmp(*argv, "-q"))
	  { // --> set the quantum
	    ASSERT(argc > 1);
	    schedTimerTicks = atoi(*(argv + 1));
	    argCount = 2;
	  }
	else if (!strcmp(*argv, "-H"))
	  { // --> set the histogram bounds
	    ASSERT(argc > 1);

	    // "Histogram::" specifies the namespace of the Histogram
	    // class. Objects of that class are histograms. When those
	    // objects are initialized, they find the number of
	    // buckets, bucket width, and minimum datum specifications
	    // in the HistoN, HistoWidth, and HistoMin data members of
	    // the class (N.B. Histo* are data members of the class,
	    // not of the objects).
	    sscanf( *(argv + 1) , "%u,%u,%i;%u,%u,%i" ,
		    & Histogram::HistoN1 , & Histogram::HistoWidth1 , & Histogram::HistoMin1 ,
		    & Histogram::HistoN2 , & Histogram::HistoWidth2 , & Histogram::HistoMin2
		    );
	    argCount = 2;
	  }
	else if (!strcmp(*argv, "-i"))
	  { // --> set the number of inertial quanta
	    ASSERT(argc > 1);
	    argCount = 2;
	  }
	else if (!strcmp(*argv, "-cw"))
	  { // --> set the number of inertial quanta
	    ASSERT(argc > 1);

	    // "Thread::" specifies the namespace of the thread
	    // histogram class. This command line argument is used to
	    // switch on the contraction of the working set.
	    Thread::wssContractionEnabled = true;

	    argCount = 1;
	  }
 	else if (!strcmp(*argv, "-R"))
 	  { // --> set the dynamic priority retention factor
 	    ASSERT(argc > 1);
 
 	    // "Thread::" specifies the namespace of the Thread
 	    // class. When Thread objects reduce their dynamic
 	    // priority advantage they find the factor to reduce it by
 	    // in the dpRetentionFactor data member of the class
 	    // (N.B. dpRetentionFactor is a data members of the class,
 	    // not of the objects--all objects share it).
 	    Thread::dpRetentionFactor = atof(*(argv + 1));
 	    argCount = 2;
 	  }
#ifdef USER_PROGRAM
#ifdef REMOTE_USER_PROGRAM_DEBUGGING
	if (!strcmp(*argv, "-gdb")) {
	    GDBRemotePort = atoi(*(argv + 1));
	} else
#endif
	if (!strcmp(*argv, "-s")) {
	    debugUserProg = true;
	} else if (!strcmp(*argv, "-S")) {
	    SwapFileName = *(argv+1);
            argCount = 2;
        } else if (!strcmp(*argv, "-noswitch")) {
            noSwitch = true;
        } else if (!strcmp(*argv, "-printstats")) {
            printProcStats = true;
        } else if (!strcmp(*argv, "-delta")) {
            ASSERT(argc > 1);
            wsDeltaSize = atoi (*(argv + 1));
            argCount = 2;
	    DSTRM_EVENT(ARGS, DELTA, wsDeltaSize);
        } else if (!strcmp(*argv, "-prp")) {
            if (!strcmp (*(argv+1), "fifo")) {
                pageReplPolicy = FIFO;
            } else if (!strcmp (*(argv+1), "lru")) {
                pageReplPolicy = LRU;
            } else if (!strcmp (*(argv+1), "secondchance")) {
                pageReplPolicy = SECONDCHANCE;
            } else if (!strcmp (*(argv+1), "dumb")) {
                pageReplPolicy = DUMB;
            } else {
                printf ("Unknown page replacement policy: %s\n", *(argv+1));
                ASSERT (false);
            }
            argCount = 2;
	    DSTRM_EVENT_DATA(ARGS, PRP, strlen(*(argv+1)),
			(sizeof(char)*strlen(*(argv+1))), *(argv+1), "print_string");
        }
#endif
#ifdef FILESYS_NEEDED
	if (!strcmp(*argv, "-f"))
	    format = true;
#endif
#ifdef NETWORK
	if (!strcmp(*argv, "-l")) {
	    ASSERT(argc > 1);
	    rely = atof(*(argv + 1));
	    argCount = 2;
	} else if (!strcmp(*argv, "-m")) {
	    ASSERT(argc > 1);
	    netname = atoi(*(argv + 1));
	    argCount = 2;
	}
#endif
	}
    DebugInit(debugArgs);			// initialize DEBUG messages
    stats = new Statistics();			// collect statistics
    interrupt = new Interrupt;			// start up interrupt handling
    scheduler = new Scheduler(schedTimerTicks);	// initialize the ready queue
    if (randomYield)				// start the timer (if needed)
      schedTimer = new Timer(SchedTimerInterruptHandler, (size_t) 0, schedTimerTicks, randomYield);
    else {
      if (!noSwitch) {
        schedTimer = new Timer(SchedTimerInterruptHandler, (size_t) 0, schedTimerTicks, false);
      }
    }



    threadToBeDestroyed = NULL;
    // We didn't explicitly allocate the current thread we are running in.
    // But if it ever tries to give up the CPU, we better have a Thread
    // object to save its state. 
    currentThread = new Thread();
    currentThread->setStatus(RUNNING);

    interrupt->Enable();
    CallOnUserAbort(Cleanup);			// if user hits ctl-C
    
#ifdef USER_PROGRAM
    machine = new Machine(debugUserProg);	// this must come first
#ifdef REMOTE_USER_PROGRAM_DEBUGGING
    if (GDBRemotePort) remoteDebugger = new GDBRemoteDebugger(GDBRemotePort);   // GDB remote debugger
#endif
#endif

#ifdef FILESYS
    synchDisk = new SynchDisk("DISK");
#endif

#ifdef FILESYS_NEEDED
    fileSystem = new FileSystem(format);
#endif

#ifdef NETWORK
    postOffice = new PostOffice(netname, rely, 10);
#endif


#ifdef USER_PROGRAM
    // Create the memory and swap managers, and a console object.
    memory = new MemoryManager();
    swap = new SwapManager();
    consoleRead = new KernelSemaphore((char *)"console read", 1);
    consoleWrite = new KernelSemaphore((char *)"console write", 1);
    consoleReadAvail = new KernelSemaphore((char *)"console read avail", 0);
    consoleWriteDone = new KernelSemaphore((char *)"console write done", 0);
    console = new Console(NULL, NULL, ConsoleReadAvail, ConsoleWriteDone, 0);
#endif
}

//----------------------------------------------------------------------
// Cleanup
// 	Nachos is halting.  De-allocate global data structures.
//----------------------------------------------------------------------
void
Cleanup()
{
    int fault_delta;
    printf("\nCleaning up...\n");
#ifdef NETWORK
    delete postOffice;
#endif
    
#ifdef USER_PROGRAM
    delete machine;
#ifdef REMOTE_USER_PROGRAM_DEBUGGING
    if (remoteDebugger) {
	remoteDebugger->Exit();
	delete remoteDebugger;
    }
#endif
#endif

#ifdef FILESYS_NEEDED
    delete fileSystem;
#endif

#ifdef FILESYS
    delete synchDisk;
#endif
    
    delete schedTimer;
    delete scheduler;
    delete interrupt;

	fault_delta = stats->userTicks - stats->ticksAtLastPageFault;
	DSTRM_EVENT(EXCEPTION, UserTicksSinceLastPageFault, fault_delta);

    // write-out the global pagefault histogram
    if ( DebugIsEnabled( (char *)"pfhisto" ) )
      {
	char fname[MAXFILENAMELENGTH + 1];
	char* prpName;

	fname[0] = '\0';

	switch( pageReplPolicy )
	  {
	  case DUMB:
	    prpName = (char *)"DUMB";
	    break;
	  case FIFO:
	    prpName = (char *)"FIFO";
	    break;
	  case LRU:
	    prpName = (char *)"LRU";
	    break;
	  case SECONDCHANCE:
	    prpName = (char *)"SC";
	    break;
	  }

	sprintf( fname , "interPageFaultTimes-%s-%s-%u.histo"
		 , mainProgramName
		 , prpName
		 , wsDeltaSize);
	stats->interPageFaultTimes.RecordDatum( stats->userTicks - stats->ticksAtLastPageFault );
	stats->interPageFaultTimes.Write( fname );
      }

    DebugCleanUp();		// clean-up DEBUG messages
    
    Exit(0);
}

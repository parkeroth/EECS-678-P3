// stats.h 
//	Data structures for gathering statistics about Nachos performance.
//
// DO NOT CHANGE -- these stats are maintained by the machine emulation
//
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef STATS_H
#define STATS_H

#pragma interface "machine/stats.h"

#include "copyright.h"
#include "histogram.h"

// The following class defines the statistics that are to be kept
// about Nachos behavior -- how much time (ticks) elapsed, how
// many user instructions executed, etc.
//
// The fields in this class are public to make it easier to update.

class Statistics {
  public:
    unsigned int totalTicks;   	// Total time running Nachos
    unsigned int idleTicks;    	// Time spent idle (no threads to run)
    unsigned int systemTicks;	// Time spent executing system code
    unsigned int userTicks;    	// Time spent executing user code
				// (this is also equal to # of
				// user instructions executed)

    unsigned int numContextSwitches;     // number of context switches
    unsigned int numVolContextSwitches;  // number of voluntary context switches
    unsigned int numInvolContextSwitches;// number of invol. context switches

    unsigned int numDiskReads;		 // number of disk read requests
    unsigned int numDiskWrites;		 // number of disk write requests
    unsigned int numConsoleCharsRead;	 // number of characters read from the 
                                         // keyboard
    unsigned int numConsoleCharsWritten; // number of characters written to 
                                         // the display
    unsigned int numPageFaults;		 // number of virtual memory page 
                                         // faults
    unsigned int numPageIns;             // number of virtual memory pageins
    unsigned int numPageOuts;            // number of virtual memory pageouts
    unsigned int numPacketsSent;	 // number of packets sent over the 
                                         // network
    unsigned int numPacketsRecvd;	 // number of packets received over 
                                         // the network

    Statistics(); 		// initialize everything to zero

    void Print();		// print collected statistics
    void ShortPrint(int);	// print process statistics


    Histogram interPageFaultTimes; // histogram of times elapsed between page faults
    unsigned int ticksAtLastPageFault;
};

// Constants used to reflect the relative time an operation would
// take in a real system.  A "tick" is a just a unit of time -- if you 
// like, a microsecond.
//
// Since Nachos kernel code is directly executed, and the time spent
// in the kernel measured by the number of calls to enable interrupts,
// these time constants are none too exact.

// FIXME: Should these be constants, or should we draw values from some
// probability distribution?

#define UserTick		1	// advance for each user-level instruction 
#define SystemTick 		10 	// advance each time interrupts are enabled
#define RotationTime 		500 	// time disk takes to rotate one sector
#define SeekTime 		500    	// time disk takes to seek past one track
#define ConsoleTime 		100	// time to read or write one character
#define NetworkTime 		100   	// time to send or receive one packet

#endif // STATS_H

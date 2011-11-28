// stats.h 
//	Routines for managing statistics about Nachos performance.
//
// DO NOT CHANGE -- these stats are maintained by the machine emulation.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#pragma implementation "machine/stats.h"

#include "copyright.h"
#include "utility.h"
#include "stats.h"
#include "nachos_dsui.h"

//----------------------------------------------------------------------
// Statistics::Statistics
// 	Initialize performance metrics to zero, at system startup.
//----------------------------------------------------------------------

Statistics::Statistics()
  : interPageFaultTimes( Histogram::HistoN1, Histogram::HistoWidth1, Histogram::HistoMin1 )
{
    totalTicks = idleTicks = systemTicks = userTicks = 0;
    numContextSwitches = numVolContextSwitches = numInvolContextSwitches = 0;
    numDiskReads = numDiskWrites = 0;
    numConsoleCharsRead = numConsoleCharsWritten = 0;
    numPageFaults = numPageIns = numPageOuts = 0;
    numPacketsSent = numPacketsRecvd = 0;

    ticksAtLastPageFault = 0;
}

//----------------------------------------------------------------------
// Statistics::Print
// 	Print performance metrics, when we've finished everything
//	at system shutdown.
//----------------------------------------------------------------------

void
Statistics::Print()
{
    printf("Ticks: total %u, idle %u, system %u, user %u\n", totalTicks, 
	idleTicks, systemTicks, userTicks);
    printf("Context switches: %u\n", numContextSwitches);
    printf("Disk I/O: reads %u, writes %u\n", numDiskReads, numDiskWrites);
    printf("Console I/O: reads %u, writes %u\n", numConsoleCharsRead, 
	numConsoleCharsWritten);
    printf("Paging: faults %u, pageins %u, pageouts %u\n", numPageFaults,
	   numPageIns, numPageOuts);
    printf("Network I/O: packets received %u, sent %u\n", numPacketsRecvd, 
	numPacketsSent);
    DSTRM_EVENT(STATS, PAGE_FAULTS, numPageFaults);
    DSTRM_EVENT(STATS, PAGE_INS, numPageIns);
    DSTRM_EVENT(STATS, PAGE_OUTS, numPageOuts);
}


//----------------------------------------------------------------------
// Statistics::ShortPrint
// 	Print performance metrics, called when a process exits
//----------------------------------------------------------------------

void
Statistics::ShortPrint(int pid)
{
    printf("\n[%d]\t%u total, %u user, %u system\n", pid,
		totalTicks, userTicks, systemTicks);
    printf("  \t%u context switches, %u voluntary, %u involuntary\n",
		numContextSwitches, numVolContextSwitches, 
		numInvolContextSwitches);
    printf("  \t%u reads, %u writes, %u faults, %u pageins, %u pageouts\n",
		numConsoleCharsRead, numConsoleCharsWritten, 
		numPageFaults, numPageIns, numPageOuts);
}


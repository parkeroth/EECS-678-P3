// -*- C++ -*-
// utility.h 
//	Miscellaneous useful definitions, including debugging routines.
//
//	The debugging routines allow the user to turn on selected
//	debugging messages, controllable from the command line arguments
//	passed to Nachos (-d).  You are encouraged to add your own
//	debugging tags.  The pre-defined debugging tags are:
//
//	"all"       -- turn on all debug messages
//   	"thread"    -- thread system
//   	"synch"     -- semaphores, locks, and conditions
//   	"interrupt" -- interrupt emulation
//   	"machine"   -- machine emulation (USER_PROGRAM)
//   	"disk"      -- disk emulation (FILESYS)
//   	"filesys"   -- file system (FILESYS)
//   	"address"   -- address spaces (USER_PROGRAM)
//   	"network"   -- network emulation (NETWORK)
//
// KU tags:
//      "rgdb"      -- remote GDB tags (REMOTE_USER_PROGRAM_DEBUGGING)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

// NOTE: e.g., the command:
//            nachos -x prog -d "thread:machine"
// will run the program "prog" with the "thread" and "machine" debug
// messages enabled

// Extended from character flags to an enumeration; Nicolas Frisby, Spring 2006.

#ifndef UTILITY_H
#define UTILITY_H

#pragma interface "threads/utility.h"

#include "copyright.h"
#include <stdlib.h>


// Principal debug flags
#define DB_ALL		"all"
#define DB_THREAD	"thread"
#define DB_SYNCH	"synch"
#define DB_INTERRUPT	"interrupt"
#define DB_MACHINE	"machine"
#define DB_DISK		"disk"
#define DB_FILESYS	"filesys"
#define DB_ADDRESS	"address"
#define DB_NETWORK	"network"
#define DB_RGDB		"remote_gdb"
#define DB_INSTRUMENT	"instrument"
#define DB_NICE      "nice"



// Miscellaneous useful routines

#define min(a,b)  (((a) < (b)) ? (a) : (b))
#define max(a,b)  (((a) > (b)) ? (a) : (b))

// Divide and either round up or down 
#define divRoundDown(n,s)  ((n) / (s))
#define divRoundUp(n,s)    (((n) / (s)) + ((((n) % (s)) > 0) ? 1 : 0))

// This declares the type "VoidFunctionPtr" to be a "pointer to a
// function taking an integer argument and returning nothing".  With
// such a function pointer (say it is "func"), we can call it like this:
//
//	(*func) (17);
//
// This is used by Thread::Fork and for interrupt handlers, as well
// as a couple of other places.

typedef void (*VoidFunctionPtr)(size_t arg); 
typedef void (*VoidNoArgFunctionPtr)(); 


// Include interface that isolates us from the host machine system library.
// Requires definition of bool, and VoidFunctionPtr
#include "sysdep.h"				

// Interface to debugging routines.

extern void DebugInit(char* tags);	// enable printing debug messages

extern bool DebugIsEnabled(char* tag); 	// Is this debug tag enabled?

extern void DEBUG (char* tag, char* format, ...);  	// Print debug message 
							// if tag is enabled

extern void DebugCleanUp();


//----------------------------------------------------------------------
// ASSERT
//      If condition is false,  print a message and dump core.
//	Useful for documenting assumptions in the code.
//
//	NOTE: needs to be a #define, to be able to print the location 
//	where the error occurred.
//----------------------------------------------------------------------
#define ASSERT(condition)                                                     \
    if (!(condition)) {                                                       \
        fprintf(stderr, "Assertion failed: line %d, file \"%s\"\n",           \
                __LINE__, __FILE__);                                          \
	fflush(stderr);							      \
        Abort();                                                              \
    }


#endif // UTILITY_H

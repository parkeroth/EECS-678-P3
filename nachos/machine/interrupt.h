// interrupt.h 
//	Data structures to emulate low-level interrupt hardware.
//
//	The hardware provides a routine (SetLevel) to enable or disable
//	interrupts.
//
//	In order to emulate the hardware, we need to keep track of all
//	interrupts the hardware devices would cause, and when they
//	are supposed to occur.  
//
//	This module also keeps track of simulated time.  Time advances
//	only when the following occur: 
//		interrupts are re-enabled
//		a user instruction is executed
//		there is nothing in the ready queue
//
//	As a result, unlike real hardware, interrupts (and thus time-slice 
//	context switches) cannot occur anywhere in the code where interrupts
//	are enabled, but rather only at those places in the code where 
//	simulated time advances (so that it becomes time to invoke an
//	interrupt in the hardware simulation).
//
//	NOTE: this means that incorrectly synchronized code may work
//	fine on this hardware simulation (even with randomized time slices),
//	but it wouldn't work on real hardware.  (Just because we can't
//	always detect when your program would fail in real life, does not 
//	mean it's ok to write incorrectly synchronized code!)
//
//  DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

/* FIXME: To avoid all of the weird casting with interrupt handlers, let's
   make an InterruptHandler parent class with a virtual handle() method.
   Specific devices will subclass this and fill in whatever data they need to
   operate, which will be accessed by their specific handle() methods.

   Will this affect the system call mechanism and, if so, how?
*/

#ifndef _MACHINE_INTERRUPT_H
#define _MACHINE_INTERRUPT_H

#pragma interface "machine/interrupt.h"

#include "copyright.h"
#include "system.h"
#include "list.h"

// Interrupts can be disabled (IntOff) or enabled (IntOn)
enum IntStatus { IntOff, IntOn };

// Nachos can be running kernel code (SystemMode), user code (UserMode),
// or there can be no runnable thread, because the ready list 
// is empty (IdleMode).
enum MachineStatus {IdleMode, SystemMode, UserMode};

// IntType records which hardware device generated an interrupt.
// In Nachos, we support a hardware timer device, a disk, a console
// display and keyboard, and a network.
enum IntType { TimerInt, DiskInt, ConsoleWriteInt, ConsoleReadInt, 
	       NetworkSendInt, NetworkRecvInt};

// The following class defines an interrupt that is scheduled
// to occur in the future.  The internal data structures are
// left public to make it simpler to manipulate.

struct PendingInterrupt {
  VoidFunctionPtr handler;	// The function (in the hardware device
				// emulator) to call when the interrupt occurs
  size_t arg;			// The argument to the function.
  unsigned int when;		// When the interrupt is supposed to fire
  IntType type;			// for debugging

  // Initialize an interrupt that will occur in the future.
  // "func" is the procedure to call when the interrupt occurs.
  // "param" is the argument to pass to the procedure
  // "time" is when (in simulated time) the interrupt is to occur
  // "kind" is the hardware device that generated the interrupt
  PendingInterrupt(VoidFunctionPtr func, size_t param, unsigned int time,
		   IntType kind)
    : handler(func), arg(param), when(time), type(kind) { }
};

// A function for determining PendingInterrupt order
extern bool interruptOrder(void *, void *);

// The following class defines the data structures for the simulation
// of hardware interrupts.  We record whether interrupts are enabled
// or disabled, and any hardware interrupts that are scheduled to occur
// in the future.

class Interrupt {
 public:
  Interrupt();				// initialize the interrupt simulation
  ~Interrupt();				// deallocate data structures

  IntStatus SetLevel(IntStatus level);	// Disable or enable interrupts 
					// and return previous setting.

  void Enable();			// Enable interrupts.
  IntStatus getLevel() {return level;}	// Return whether interrupts
					// are enabled or disabled

  void Idle();				// The ready queue is empty, roll 
					// simulated time forward until the 
					// next interrupt

  void Halt();				// quit and print out stats

  void YieldOnReturn();			// cause a context switch on return 
					// from an interrupt handler

  MachineStatus getStatus() { return status; } // idle, kernel, user
  void setStatus(MachineStatus st) { status = st; }

  void setNeedResched();		// Need to run the scheduler

  void DumpState();			// Print interrupt state

  // NOTE: the following are internal to the hardware simulation code.
  // DO NOT call these directly.  I should make them "private",
  // but they need to be public since they are called by the
  // hardware device simulators.

  void Schedule(VoidFunctionPtr handler,// Schedule an interrupt to occur
		size_t arg, unsigned int when, IntType type);
					// at time ``when''.  This is called
    					// by the hardware device simulators.

  void OneTick();       		// Advance simulated time

 private:
  IntStatus level;		// are interrupts enabled or disabled?
  SortedList pending;		// the list of interrupts scheduled
				// to occur in the future
  bool inHandler;		// true if we are running an interrupt handler
  bool yieldOnReturn;		// true if we are to context switch
				// on return from the interrupt handler
  MachineStatus status;		// idle, kernel mode, user mode
  bool needResched;		// true if we need to run the scheduler
				// at the next opportunity

  // these functions are internal to the interrupt simulation code
  bool HandleIfDue(bool advanceClock);  // Check and Handle if an interrupt is 
					// supposed to occur now

  void ChangeLevel(IntStatus now);      // SetLevel, without advancing the
                                        // simulated time

};

#endif // _MACHINE_INTERRRUPT_H

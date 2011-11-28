// interrupt.cc 
//	Routines to simulate hardware interrupts.
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
//  DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#pragma implementation "machine/interrupt.h"

#include "copyright.h"
#include "interrupt.h"
#include "system.h"
#include "nachos_dsui.h"

// String definitions for debugging messages

static char *intLevelNames[] = {(char *) "off", (char *)"on"};
static char *intTypeNames[] = {
  (char *)"timer", (char *)"disk", (char *)"console write", 
  (char *)"console read", (char *)"network send",
  (char *)"network recv" };

//----------------------------------------------------------------------
// Interrupt::Interrupt
//     Init data structures needed by the interrupt simulation.
//----------------------------------------------------------------------

Interrupt::Interrupt()
{

  level         = IntOff;
  inHandler     = false;
  yieldOnReturn = false;
  status        = SystemMode;
  needResched   = false;
  pending.setComparator (interruptOrder);
}

//----------------------------------------------------------------------
// Interrupt::set_needResched
//     set the flag that will cause a scheduling decision to be made
//----------------------------------------------------------------------
void
Interrupt::setNeedResched()
{
  needResched   = true;
}

//----------------------------------------------------------------------
// Interrupt::~Interrupt
// 	Deallocate the data structures needed by the interrupt simulation.
//----------------------------------------------------------------------

Interrupt::~Interrupt()
{
  while (!pending.IsEmpty())
    pending.Remove();
}

//----------------------------------------------------------------------
// Interrupt::ChangeLevel
// 	Change interrupts to be enabled or disabled, without advancing 
//	the simulated time (normally, enabling interrupts advances the time).
//
//	Used internally.
//
//	"now" -- the new interrupt status
//----------------------------------------------------------------------

void
Interrupt::ChangeLevel(IntStatus now)
{
    level = now;
    DEBUG( (char *)DB_INTERRUPT ,(char *)"\tinterrupts:  %s\n",intLevelNames[now]);
}


//----------------------------------------------------------------------
// Interrupt::SetLevel
// 	Change interrupts to be enabled or disabled, and if interrupts
//	are being enabled, advance simulated time by calling OneTick().
//
// Returns:
//	The old interrupt status.
// Parameters:
//	"now" -- the new interrupt status
//----------------------------------------------------------------------

IntStatus
Interrupt::SetLevel(IntStatus now)
{
    IntStatus old = level;
    
    ASSERT(now == IntOff || !inHandler);	// interrupt handlers are 
						// prohibited from enabling 
						// interrupts
    ChangeLevel(now);				// change to new state
    if ((now == IntOn) && (old == IntOff))
	    // When we make the transition from interrupts off to interrupts on
	    // we are moving from executing simulator code (C++) to user level 
	    // Mips instructions. Since the simulated clock is not advanced during
	    // execution of C++ code we advance it one tick here as a minimal recognition
	    // that something has happened. Note that simulated device activity does 
	    // have an effect on the timeline in the form of scheduled interrupts
	    // representing device activity completion. However, the execution of some 
	    // simulated code is not represented on the simulated timeline such as semaphores.
	OneTick();				
    return old;
}

//----------------------------------------------------------------------
// Interrupt::Enable
// 	Turn interrupts on.  Who cares what they used to be? 
//	Used in ThreadRoot, to turn interrupts on when first starting up
//	a thread.
//----------------------------------------------------------------------
void
Interrupt::Enable()
{ 
  (void) SetLevel(IntOn); 
}

//----------------------------------------------------------------------
// Interrupt::OneTick
// 	Advance simulated time and check if there are any pending 
//	interrupts to be called. 
//
//	Two things can cause OneTick to be called:
//		interrupts are re-enabled
//		an instruction on the simulated Mips CPU is executed
//              Note that user level code is the only code that is compiled
//              to Mips instructions. All OS level code is written in C++
//              and does not advance the simulated time.
//----------------------------------------------------------------------
void
Interrupt::OneTick()
{
    MachineStatus old = status;
    
    // advance simulated time
    if (status == SystemMode) {
        stats->totalTicks += SystemTick;
	stats->systemTicks += SystemTick;
        if (currentThread) {
            currentThread->procStats->totalTicks += SystemTick;
            currentThread->procStats->systemTicks += SystemTick;
        }
    } else {					// USER_PROGRAM
	stats->totalTicks += UserTick;
	stats->userTicks += UserTick;
        if (currentThread) {
            currentThread->procStats->totalTicks += UserTick;
            currentThread->procStats->userTicks += UserTick;
        }
    }


    DEBUG( (char *)DB_INTERRUPT , (char *)"\n== Tick %d ==\n", stats->totalTicks);

    // check any pending interrupts are now ready to fire
    ChangeLevel(IntOff);		// first, turn off interrupts
					// (interrupt handlers run with
					// interrupts disabled)

    while (HandleIfDue(false))		// Check & Handle pending interrupts
	;

    ChangeLevel(IntOn);			// re-enable interrupts
    // if the timer device handler asked
    // for a context switch, or if something
    // made us think the a scheduling decision
    // should run, then  ok to do it now
    //
    if (yieldOnReturn || needResched) {
	needResched = false;
	yieldOnReturn = false;
 	status = SystemMode;		// yield is a kernel routine
	currentThread->Yield();
	status = old;
    }
}

//----------------------------------------------------------------------
// Interrupt::YieldOnReturn
// 	Called from within an interrupt handler, to cause a context switch
//	(for example, on a time slice) in the interrupted thread,
//	when the handler returns.
//
//	We can't do the context switch here, because that would switch
//	out the interrupt handler, and we want to switch out the 
//	interrupted thread.
//----------------------------------------------------------------------

void
Interrupt::YieldOnReturn()
{ 
    ASSERT(inHandler);  
    yieldOnReturn = true; 
    wasYieldOnReturn = true;
}

//----------------------------------------------------------------------
// Interrupt::Idle
// 	Routine called when there is nothing in the ready queue.
//
//	Since something has to be running in order to put a thread
//	on the ready queue, the only thing to do is to advance 
//	simulated time until the next scheduled hardware interrupt.
//
//	If there are no pending interrupts, stop.  There's nothing
//	more for us to do.
//----------------------------------------------------------------------
void
Interrupt::Idle()
{
    unsigned int when;
    DEBUG( (char *)DB_INTERRUPT , (char *)"Machine idling; checking for interrupts.\n");
    status = IdleMode;
    if (HandleIfDue(true)) {
	PendingInterrupt *toOccur =
	  static_cast<PendingInterrupt *>(pending.Head());
	when = toOccur->when;
	stats->idleTicks += (when - stats->totalTicks);
	stats->totalTicks = when;
	while (HandleIfDue(false))
	    ;
	yieldOnReturn = false;		// Set this to false to add robustness
					// because the idle thread will always
					// yield to any other
	status = SystemMode;		// Switching back to the System mode
					// from Idle mode
	return;				// return in case there's now
					// a runnable thread
    }
    // if there are no pending interrupts, and nothing is on the ready
    // queue, it is time to stop.   If the console or the network is 
    // operating, there are *always* pending interrupts, so this code
    // is not reached.  Instead, the halt must be invoked by the user program.
    DEBUG( (char *)DB_INTERRUPT , (char *)"Machine idle.  No interrupts to do.\n");
    printf("No threads ready or runnable, and no pending interrupts.\n");
    printf("Assuming the program completed.\n");
    Halt();
}

//----------------------------------------------------------------------
// Interrupt::Halt
// 	Shut down Nachos cleanly, printing out performance statistics.
//----------------------------------------------------------------------
void
Interrupt::Halt()
{
    printf("Machine halting!\n\n");
    stats->Print();
    DSUI_CLEANUP();
    Cleanup();     // Never returns.
}

//----------------------------------------------------------------------
// Interrupt::Schedule
// 	Arrange for the CPU to be interrupted when simulated time
//	reaches "now + when".
//
//	Implementation: just put it on a sorted list.
//
//	NOTE: the Nachos kernel should not call this routine directly.
//	Instead, it is only called by the hardware device simulators.
//
//	"handler" is the procedure to call when the interrupt occurs
//	"arg" is the argument to pass to the procedure
//	"fromNow" is how far in the future (in simulated time) the 
//		 interrupt is to occur
//	"type" is the hardware device that generated the interrupt
//----------------------------------------------------------------------
// FIXME: Make arg a union instead of a size_t to avoid evil casts
void
Interrupt::Schedule(VoidFunctionPtr handler, size_t arg, unsigned int fromNow,
		    IntType type)
{
  IntStatus oldLevel;
  unsigned int when = stats->totalTicks + fromNow;

  PendingInterrupt *toOccur = new PendingInterrupt(handler, arg, when, type);
  
  DEBUG( (char *)DB_INTERRUPT , (char *)"Scheduling interrupt handler the %s at time = %u\n", 
	intTypeNames[type], when);
  ASSERT(fromNow > 0);
  
  oldLevel = SetLevel (IntOff);
  pending.Insert(toOccur);
  SetLevel (oldLevel);
}


//----------------------------------------------------------------------
// Interrupt::CheckIfDue
// 	Check if an interrupt is scheduled to occur, and if so, fire it off.
//
// Returns:
//	true, if we fired off any interrupt handlers
// Params:
//	"advanceClock" -- if true, there is nothing in the ready queue,
//		so we should simply advance the clock to when the next 
//		pending interrupt would occur (if any).  If the pending
//		interrupt is just the time-slice daemon, however, then 
//		we're done!
//----------------------------------------------------------------------
bool
Interrupt::HandleIfDue(bool advanceClock)
{
    MachineStatus old = status;
    unsigned int when;
    PendingInterrupt *toOccur;

    ASSERT(level == IntOff);		// interrupts need to be disabled,
					// to invoke an interrupt handler
    if (DebugIsEnabled((char *)"interrupt"))
	DumpState();
 
    toOccur = static_cast<PendingInterrupt *>(pending.Head());
    when = toOccur->when;
    
    if (toOccur == NULL)		// no pending interrupts
	return false;			 

    if (when > stats->totalTicks) {
        return advanceClock;
    } else {
	/* Remove the interrupt we peeked at when we set the value of toOccur
	   above, because it is time to handle the interrupt */
	(void) pending.Remove();
    }

    // Check if there is nothing more to do, and if so, quit
    if (status == IdleMode && toOccur->type == TimerInt && pending.IsEmpty()) {
	pending.Insert(toOccur);
	return false;
    }
    DEBUG((char *) DB_INTERRUPT ,(char *) "Invoking interrupt handler for the %s at time %u\n", 
			intTypeNames[toOccur->type], toOccur->when);
#ifdef USER_PROGRAM
    // Any machine instruction followed by a delay slot which causes a
    // trap/interrupt will be restarted, and therefore the instruction in the
    // delay slot will be executed again.  Therefore, we must cancel any
    // pending delay slot instructions.  An example of this is: if the load
    // instruction being executed causes a page fault, then we have to cancel
    // it until the page fault handler has done its job, then restart the load.
    if (machine != NULL)
    	machine->DelayedLoad(0, 0);
#endif
    inHandler = true;
    status = SystemMode;			// whatever we were doing,
						// we are now going to be
						// running in the kernel
    (*(toOccur->handler))(toOccur->arg);	// call the interrupt handler
    status = old;				// restore the machine status
    inHandler = false;
    delete toOccur;
    if (pending.IsEmpty()) {
	return false;
    } else {
	PendingInterrupt *next =
	  static_cast<PendingInterrupt *>(pending.Head());
	return next->when == when;
    }
}

//----------------------------------------------------------------------
// interruptOrder
// 	Determine in which order two interrupts should occur.
//----------------------------------------------------------------------
bool
interruptOrder (void *int1, void *int2)
{
  return ((PendingInterrupt *) int1)->when < ((PendingInterrupt *) int2)->when;
}

//----------------------------------------------------------------------
// PrintPending
// 	Print information about an interrupt that is scheduled to occur.
//	When, where, why, etc.
//----------------------------------------------------------------------

static void
PrintPending(size_t arg)
{
    PendingInterrupt *pend = (PendingInterrupt *)arg;

    printf("Interrupt handler %s, scheduled at %u\n", 
	intTypeNames[pend->type], pend->when);
}

//----------------------------------------------------------------------
// DumpState
// 	Print the complete interrupt state - the status, and all interrupts
//	that are scheduled to occur in the future.
//----------------------------------------------------------------------

void
Interrupt::DumpState()
{
    printf("Time: %d, interrupts %s\n", stats->totalTicks, 
					intLevelNames[level]);
    printf("Pending interrupts:\n");
    fflush(stdout);
    pending.Mapcar(PrintPending);
    printf("End of pending interrupts\n");
    fflush(stdout);
}

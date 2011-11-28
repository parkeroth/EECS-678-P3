// breakpoint.h: implementation of breakpoints and watchpoints for Nachos.
//
// Written by Jerry James <james@eecs.ku.edu>, 2001

#ifndef _MACHINE_BREAKPOINT_H
#define _MACHINE_BREAKPOINT_H

#pragma interface "machine/breakpoint.h"

#include "utility.h"

// These are the types that GDB knows about
enum BreakpointType {
  SoftwareBp,	// Software breakpoint; munge the code
  HardwareBp,	// Hardware breakpoint
  WriteWp,	// Write watchpoint
  ReadWp,	// Read watchpoint
  AccessWp	// Access watchpoint
};

struct Breakpoint {
  int start;			// Start location of the breakpoint
  int end;			// End location of the breakpoint
  enum BreakpointType type;	// What kind of break/watchpoint?
};

#define BP_SET_INIT 64
#define BP_SET_INC  32

// A set of breakpoints, in searchable form
class BreakpointSet {
 private:
  Breakpoint *set;	// The set of Breakpoint objects
  int size;		// The size of the array
  int count;		// The number of entries in the array

  int findAddr(const int address);

 public:
  BreakpointSet() : set(new Breakpoint[BP_SET_INIT]), size(BP_SET_INIT),
		    count(0) { }

  ~BreakpointSet() { delete[] set; }

  void addBreakpoint(const int address, const int length,
		     const enum BreakpointType type);
  void removeBreakpoint(const int address, const int length,
			const enum BreakpointType type);
  bool queryBreakpoint(const int address, const int length,
		       const enum BreakpointType type, int &addr);
};

#endif /* _MACHINE_BREAKPOINT_H */

// breakpoint.cc: implement break/watchpoints for Nachos
//
// Note that there is no different between hardware and software breakpoints
// since this is a simulator.  Read, write, and access watchpoints are all
// implemented.
//
// Written by Jerry James <james@eecs.ku.edu>, 2001
// Copyright (c) 2001, 2003 KU Center for Research (KUCR)

/* FIXME: Use one of the STL container classes instead of the custom array */

#pragma implementation "machine/breakpoint.h"

#include "breakpoint.h"

// findAddr: do a binary search on the array
// Return either:
//   (a) the index of the first range containing address, if there is one; or
//   (b) the index of the first range beyond address, otherwise.
int
BreakpointSet::findAddr(const int address) {
  int low = 0, high = count - 1;

  while (low <= high)
    {
      int mid = (low + high) / 2;
      if (set[mid].start <= address && address <= set[mid].end)
	{
	  low = mid;
	  break;
	}
      else if (address < set[mid].start)
	high = mid - 1;
      else
	low = mid + 1;
    }

  // In case this address is in more than 1 range (what does that mean?), back
  // up to the first one
  while (low > 0 && set[low - 1].start <= address &&
	 address <= set[low - 1].end)
    low--;

  return low;
}

// addBreakpoint: activate a breakpoint or watchpoint (TYPE) at ADDRESS,
// covering LENGTH bytes
void
BreakpointSet::addBreakpoint(const int address, const int length,
			     const enum BreakpointType type) {
  // Make sure the array is big enough to add one
  if (size == count)
    {
      Breakpoint *newSet;

      newSet = new Breakpoint[size + BP_SET_INC];
      memcpy(newSet, set, size * sizeof(Breakpoint));
      delete[] set;
      set = newSet;
      size += BP_SET_INC;
    }

  // At this point, count < size, but make sure...
  ASSERT(count < size);

  int ind = findAddr(address);	// This is the index for the new bp
  if (ind < count)
    memmove(&set[ind + 1], &set[ind], (count-ind) * sizeof(Breakpoint));
  count++;

  // Set the new breakpoint
  set[ind].start = address;
  set[ind].end = address + length - 1;
  set[ind].type = type;
}

// removeBreakpoint: deactivate a breakpoint or watchpoitn (TYPE) at ADDRESS,
// covering LENGTH bytes
void
BreakpointSet::removeBreakpoint(const int address, const int length,
				const enum BreakpointType type) {
  for (int ind = findAddr(address);
       ind < count && set[ind].start <= address && address <= set[ind].end;
       ind++)
    {
      if (set[ind].start == address && set[ind].end == address + length - 1 &&
	  set[ind].type == type)
	{
	  // This is the one to remove
	  count--;
	  if (ind < count)
	    memmove(&set[ind], &set[ind + 1],
		    (count - ind) * sizeof(Breakpoint));
	}
    }
}

// queryBreakpoint: return true if there is a breakpoint of type TYPE covering
// [ADDRESS, ADDRESS + LENGTH - 1].  If so, store the starting address of the
// breakpoint or watchpoint in BREAKADDR.
bool
BreakpointSet::queryBreakpoint(const int address, const int length,
			       const enum BreakpointType type,
			       int &breakAddr) {
  register int ind = findAddr(address);

  switch (type)
    {
    case SoftwareBp:
    case HardwareBp:
      for (; ind < count && set[ind].start <= address + length - 1;
	   ind++)
	if (set[ind].type == SoftwareBp || set[ind].type == HardwareBp)
	  {
	    breakAddr = address < set[ind].start ? set[ind].start : address;
	    return true;
	  }
      break;

    case WriteWp:
      for (; ind < count && set[ind].start <= address + length - 1;
	   ind++)
	if (set[ind].type == WriteWp || set[ind].type == AccessWp)
	  {
	    breakAddr = address < set[ind].start ? set[ind].start : address;
	    return true;
	  }
      break;

    case ReadWp:
      for (; ind < count && set[ind].start <= address + length - 1;
	   ind++)
	if (set[ind].type == ReadWp || set[ind].type == AccessWp)
	  {
	    breakAddr = address < set[ind].start ? set[ind].start : address;
	    return true;
	  }
      break;

    case AccessWp:
      for (; ind < count && set[ind].start <= address + length - 1;
	   ind++)
	if (set[ind].type == AccessWp)
	  {
	    breakAddr = address < set[ind].start ? set[ind].start : address;
	    return true;
	  }
      break;

    default:
      break;
    }

  return false;
}

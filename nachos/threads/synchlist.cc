// synchlist.cc
//	Routines for synchronized access to a list.
//
//	Implemented by surrounding the List abstraction
//	with synchronization routines.
//
// 	Implemented in "monitor"-style -- surround each procedure with a
// 	lock acquire and release pair, using condition signal and wait for
// 	synchronization.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#pragma implementation "threads/synchlist.h"

#include "copyright.h"
#include "synchlist.h"

//----------------------------------------------------------------------
// SynchList::SynchList
//	Allocate and initialize the data structures needed for a 
//	synchronized list, empty to start with.
//	Elements can now be added to the list.
//----------------------------------------------------------------------

SynchList::SynchList()
{
  lock = new Lock ((char *)"list lock"); 
  listEmpty = new Condition ((char *)"list empty cond");
}


//----------------------------------------------------------------------
// SynchList::~SynchList
//	Deallocate the data structures created for synchronizing a list. 
//----------------------------------------------------------------------

SynchList::~SynchList()
{ 
  delete lock;
  delete listEmpty;
}


//----------------------------------------------------------------------
// SynchList::Prepend
//      Put an "item" on the front of the list and wake up any threads
//      waiting for an element to be put on the list
//
//	"item" is the thing to put on the list
//----------------------------------------------------------------------

void SynchList::Prepend(void *item)
{
    lock->Acquire ();		// enforce mutual exclusive access to the list 
    List::Prepend (item);
    listEmpty->Signal (lock);	// wake up a waiter, if any
    lock->Release ();
}


//----------------------------------------------------------------------
// SynchList::Append
//      Append an "item" to the end of the list.  Wake up any threads
//	waiting for an element to be appended.
//
//	"item" is the thing to put on the list
//----------------------------------------------------------------------

void SynchList::Append(void *item)
{
    lock->Acquire ();		// enforce mutual exclusive access to the list 
    List::Append (item);
    listEmpty->Signal (lock);	// wake up a waiter, if any
    lock->Release ();
}


//----------------------------------------------------------------------
// SynchList::Remove
//      Remove an "item" from the beginning of the list.  Wait if
//	the list is empty.
// Returns:
//	The removed item. 
//----------------------------------------------------------------------

void *SynchList::Remove()
{
  void *item;

  lock->Acquire ();			// enforce mutual exclusion
  while (IsEmpty())
    listEmpty->Wait (lock);		// wait until list isn't empty
  item = List::Remove ();
  ASSERT (item != NULL);
  lock->Release ();
  return item;
}


//----------------------------------------------------------------------
// SynchList::Mapcar
//      Apply function to every item on the list.  Obey mutual exclusion
//	constraints.
//
//	"func" is the procedure to be applied.
//----------------------------------------------------------------------

void SynchList::Mapcar(VoidFunctionPtr func)
{ 
  lock->Acquire (); 
  List::Mapcar (func);
  lock->Release (); 
}

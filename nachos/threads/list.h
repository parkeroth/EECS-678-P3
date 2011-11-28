// -*- C++ -*-
// list.h 
//	Data structures to manage LISP-like lists.  
//
//      As in LISP, a list can contain any type of data structure
//	as an item on the list: thread control blocks, 
//	pending interrupts, etc.  That is why each item is a "void *",
//	or in other words, a "pointers to anything".
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//
// 12 Mar 2004: Changed by Jerry James to support multiple List types.

#ifndef LIST_H
#define LIST_H

#pragma interface "threads/list.h"

#include "copyright.h"
#include "utility.h"

// The following class defines a "list element" -- which is
// used to keep track of one item on a list.  It is equivalent to a
// LISP cell, with a "cdr" ("next") pointing to the next element on the list,
// and a "car" ("item") pointing to the item on the list.

class ListElement {
public:
  ListElement(void *itemPtr);	// Constructor

  ListElement *next;		// next element on list, NULL if this is last
  void *item;			// the actual item
};

// The following class defines a "list" -- a singly linked list of
// list elements, each of which points to a single item on the list.

class List {
public:
  List();			// allocate a list
  ~List();			// deallocate a list

  bool IsEmpty() const;		// Return true if the list is empty
  void Prepend(void *item);	// Add an item at the beginning of the list
  void Append(void *item);	// Add an item at the end of the list
  void *Remove();		// Take item off the front of the list
  void *Remove(void *item);	// Remove a particular item from the list
  void Delete(void *item);	// Remove a particular item from the list
  void *Head() const;			// Look at the first element in the list
  void *Tail() const;			// Look at the last element in the list
  void Mapcar(VoidFunctionPtr func) const;	// Apply "func" to every element
  				// on the list

protected:
  ListElement *first;		// Head of the list, NULL if list is empty
  ListElement *last;		// Last element of list, NULL if empty
};

// The following class defines a sorted list, where the user has to provide a
// comparison function to tell which items are "greater" than others.  The
// sorting function should look like this:
//
// bool mySortFunction(void *item1, void *item2) {
//   if (item1 should be closer to the head of the list than item2)
//     return true;
//   else
//     return false;
// }

typedef bool (*listItemComparator)(void *, void *);

//class SortedList derives from the parent class List
//and defines new member functions

class SortedList : public List {
public:
  void setComparator (listItemComparator comp);
  void Insert (void *item);	// Insert an element in sorted order
private:
  listItemComparator compare;
};

#endif // LIST_H

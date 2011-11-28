// list.cc 
//
//     	Routines to manage a singly-linked list of "things".
//
// 	A "ListElement" is allocated for each item to be put on the
//	list; it is deallocated when the item is removed.  This means
//      we don't need to keep a "next" pointer in every object we
//      want to put on a list.
// 
//     	NOTE: Mutual exclusion must be provided by the caller.
//  	If you want a synchronized list, you must use the routines 
//	in synchlist.cc.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#pragma implementation "threads/list.h"

#include <iostream>
#include "copyright.h"
#include "list.h"

//----------------------------------------------------------------------
// ListElement::ListElement
// Initializes a list element
//----------------------------------------------------------------------

ListElement::ListElement(void *itemPtr)
{
  next = NULL;
  item = itemPtr;
}


//----------------------------------------------------------------------
// List::List
// Initializes a list
//----------------------------------------------------------------------

List::List()
{
  first = NULL;
  last = NULL;
}

//----------------------------------------------------------------------
// List::~List
//	Prepare a list for deallocation.  If the list still contains any 
//	ListElements, deallocate them.  However, note that we do *not*
//	deallocate the "items" on the list -- this module allocates
//	and deallocates the ListElements to keep track of each item,
//	but a given item may be on multiple lists, so we can't
//	deallocate them here.
//----------------------------------------------------------------------

List::~List()
{ 
  while (Remove() != NULL)
    ;	 // delete all the list elements
}

//----------------------------------------------------------------------
// List::IsEmpty
//      Return 'true' if the list has no elements, 'false' otherwise
//----------------------------------------------------------------------

bool List::IsEmpty () const
{
  return first == NULL;
}

//----------------------------------------------------------------------
// List::Prepend
//      Put an "item" on the front of the list.
//      
//	Allocate a ListElement to keep track of the item.
//      If the list is empty, then this will be the only element.
//	Otherwise, put it at the beginning.
//
//	"item" is the thing to put on the list
//----------------------------------------------------------------------

void List::Prepend (void *item)
{
  ListElement *element = new ListElement (item);

  if (IsEmpty())		// list is empty
    {
      first = element;
      last = element;
    }
  else
    {				// else put it before first
      element->next = first;
      first = element;
    }
}


//----------------------------------------------------------------------
// List::Append
//      Append an "item" to the end of the list.
//      
//	Allocate a ListElement to keep track of the item.
//      If the list is empty, then this will be the only element.
//	Otherwise, put it at the end.
//
//	"item" is the thing to put on the list
//----------------------------------------------------------------------

void List::Append(void *item)
{
  ListElement *element = new ListElement (item);

  if (IsEmpty()) {		// list is empty
    first = element;
    last = element;
  } else {			// else put it after last
    last->next = element;
    last = element;
  }
}

//----------------------------------------------------------------------
// List::Remove
//      Remove the first "item" from the front of the list.
// 
// Returns:
//	Pointer to removed item, NULL if nothing on the list.
//----------------------------------------------------------------------

void *List::Remove ()
{
  ListElement *element = first;
  void *thing;

  if (IsEmpty ())
    {
      return NULL;
    }

  thing = first->item;
  if (first == last)	// list had one item, now has none
    {
      first = NULL;
      last = NULL;
    }
  else
    {
      first = first->next;
    }
  delete element;
  return thing;
}

//----------------------------------------------------------------------
// List::Remove
//      Remove a specific item from the list.
// 
//----------------------------------------------------------------------

void *List::Remove (void *item)
{
  ListElement *temp, *prev;

  // Search the list for the item
  for (temp = first, prev = NULL; temp != NULL; temp = temp->next)
    {
      // Did we find it?
      if (temp->item == item)
	{
	  // We found it; unlink it from the list
	  if (prev == NULL)
	    {
	      first = temp->next;
	    }
	  else
	    {
	      prev->next = temp->next;
	    }
	  if (temp == last)
	    {
	      last = prev;
	    }
	  void* retval = temp->item;
	  delete temp;
	  return retval;
	}
      prev = temp;
    }

  // Didn't find it
  return NULL;
}


// ---------------------------------------------------------------------
// List::Delete
//      Delete an item from the list
// ---------------------------------------------------------------------

void List::Delete (void *item)
{
  // Remove it and discard the return value
  (void) Remove (item);
}

//----------------------------------------------------------------------
// List::Head
//      Return the first element in the list
//----------------------------------------------------------------------

void *List::Head () const
{
  return (first == NULL) ? NULL : first->item;
}

//----------------------------------------------------------------------
// List::Tail
//      Return the last element in the list
//----------------------------------------------------------------------

void *List::Tail () const
{
  return (last == NULL) ? NULL : last->item;
}

//----------------------------------------------------------------------
// List::Mapcar
//	Apply a function to each item on the list, by walking through  
//	the list, one element at a time.
//
//	Unlike LISP, this mapcar does not return anything!  (So it should be
//	called mapc, which is the LISP function that does this without
//	returning anything!)
//
//	"func" is the procedure to apply to each element of the list.
//----------------------------------------------------------------------

void List::Mapcar (VoidFunctionPtr func) const
{
  for (ListElement *ptr = first; ptr != NULL; ptr = ptr->next)
    {
      (*func)((size_t) ptr->item);
    }
}





//----------------------------------------------------------------------
// SortedList::setComparator
// Set the comparison function for a sorted list
//----------------------------------------------------------------------

void
SortedList::setComparator (listItemComparator comp)
{
  compare = comp;
}

//----------------------------------------------------------------------
// SortedList::Insert
//      Insert an "item" into a list, so that the list elements are
//	sorted in order.
//
//	Allocate a ListElement to keep track of the item.
//      If the list is empty, then this will be the only element.
//	Otherwise, walk through the list, one element at a time,
//	to find where the new item should be placed.
//
//	"item" is the thing to put on the list
//----------------------------------------------------------------------

void SortedList::Insert (void *item)
{
  ListElement *element = new ListElement (item);

  if (IsEmpty ()) {		// if list is empty, item is first & last
    first = element;
    last = element;
  } else if (compare (item, first->item)) {
    element->next = first;	// item goes on front of list
    first = element;
  } else {			// look for first elt in list bigger than item
    for (ListElement *ptr = first; ptr->next != NULL; ptr = ptr->next) {
      if (compare (item, ptr->next->item)) {
	element->next = ptr->next;
	ptr->next = element;
	return;
      }
    }
    last->next = element;	// item goes at end of list
    last = element;		// and update last to reflect the new element
  }
}




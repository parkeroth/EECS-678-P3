// $Id: memmgr.h,v 1.1.1.1 2003/10/15 19:43:00 james Exp $
// 
// This object manages the memory resources of the simulated MIPS processor for
// the Nachos kernel. It keeps track of which pages of memory are free, and 
// which are empty, and provides interface functions to access this
// information.
//
// Authors: Matt Peters, Doug McClendon, Shyam Pather

#ifdef USER_PROGRAM

#ifndef __MEMMGR_H
#define __MEMMGR_H

#pragma interface "userprog/memmgr.h"

#include "bitmap.h"
#include "machine.h"

class AddrSpace;

struct Frame {          // VIRTUAL_MEMORY
  AddrSpace **owners;
  int numOwners;
  int owners_page_number;
};


#include "addrspace.h"
#include "thread.h"
class Thread;


//----------------------------------------------------------------------------
// Description of class MemoryManager member functions:
//
// MemoryManager::MemoryManager() 
//   This is the default constructor for the class. It simply clears all the
//   page flags, indicating that all pages are free.
//
// MemoryManager::~MemoryManager() 
//   This is the default destructor. It does nothing for now.
//
// MemoryManager::get_next_free_page
//   Gets the number of the next free page in memory. If there are no free
//   pages, it sets nerrno to ENOMEM, and returns -1.
// 
//   Return value:
//   Normal                : The number of the next free page
//   Error (no free pages) : -1
//
//  MemoryManager::release_page
//    Mark a used page as free. This function is called when a process no 
//    longer needs a memory page that it owns (i.e. during cleanup). 
//
//    Arguments:
//    page_num : The number of the page to release.
//---------------------------------------------------------------------------

class MemoryManager {
public:
  MemoryManager();          // Clears all the bits in the page_flags bitmap
  ~MemoryManager();         // do-nothing 

  int get_next_free_page(); // returns the number of the next free page
  // called to free a page that was used 
  void release_page(int page_num, AddrSpace *addrspace); 

  void pagein( int page_number, AddrSpace * addrspace );
  void pageout( int victim );
  Frame *get_frame( int number );
  int add_frame_owner ( int frame_number, Thread * thread, int owner_page );
  int getNumOwners (int number) {
    if ((number > NumPhysPages) || (number < 0)) {
      return -1;
    }
    return Frames[number].numOwners;
  }

  int Choose_Victim (int notMe);
  int Dumb_Choose_Victim (int notMe);

private:
  BitMap *page_flags;
  
  Frame Frames[NumPhysPages];  // array of information on each frame
  // so we can do virtual memory. VIRTUAL_MEMORY
};

#endif
#endif

// $Id: swapmgr.h,v 1.1.1.1 2003/10/15 19:43:00 james Exp $
// 
// This object manages the memory resources of the simulated MIPS processor for
// the Nachos kernel. It keeps track of which pages of memory are free, and 
// which are empty, and provides interface functions to access this
// information.
//
// Authors: Matt Peters, Doug McClendon, Shyam Pather

#ifdef USER_PROGRAM

#ifndef __SWAPMGR_H
#define __SWAPMGR_H

#pragma interface "userprog/swapmgr.h"

#include "bitmap.h"
#include "memmgr.h"
#include <machine.h>

#define NumSwapPages 4096
extern char *SwapFileName;


//----------------------------------------------------------------------------
// Description of class SwapManager member functions:
//
// SwapManager::SwapManager() 
//   This is the default constructor for the class. It simply clears all the
//   frame flags, indicating that all frames are free.
//
// SwapManager::~SwapManager() 
//   This is the default destructor. It does nothing for now.
//
// SwapManager::get_next_free_frame
// 
//   Return value:
//   Normal                : The number of the next free frame
//   Error (no free frames): -1
//
//  SwapManager::release_frame
//
//    Arguments:
//    offset : the offset of the frame to release
//---------------------------------------------------------------------------

class SwapManager {
public:
  SwapManager();          // Clears all the bits in the page_flags bitmap
  ~SwapManager();         // do-nothing 

  int get_next_free_frame(); // returns the number of the next free page
  // called to free a page that was used 
  void release_frame(size_t offset, AddrSpace *addrspace); 
  Frame * get_frame(size_t offset);
  int add_frame_owner( size_t offset, Thread * thread, int owner_page );
  OpenFile *file () {
    return swapFile;
  }
  int getNumOwners (int number) {
    if ((number > NumSwapPages) || (number < 0)) {
      return -1;
    }
    return Frames[number].numOwners;
  }

private:
  BitMap *frame_flags;
  OpenFile *swapFile;
  Frame Frames[NumSwapPages];  // array of information on each frame
                               // so we can do virtual memory. VIRTUAL_MEMORY
};

#endif
#endif

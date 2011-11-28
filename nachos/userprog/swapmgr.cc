// $Id: swapmgr.cc,v 1.2 2009/02/15 22:16:10 mjantz Exp $
//
// This is the implementation of the Memory Manager object.
//
// Authors: Matt Peters, Doug McClendon, Shyam Pather

#ifdef USER_PROGRAM

#pragma implementation "userprog/swapmgr.h"

#include "swapmgr.h"
#include "nerrno.h"
#include "system.h"
#include "machine.h"
#include <unistd.h>

char *SwapFileName = (char *)"swapfile";

SwapManager::SwapManager() {
  //
  // Clear all the frame flags. This means that all frames are free.
  //
  frame_flags = new BitMap(NumSwapPages);
  for (int i = 0; i < NumSwapPages; i++) {
    frame_flags->Clear(i);
    Frames[i].owners = NULL;
    Frames[i].numOwners = 0;
  }

  fileSystem->Create(SwapFileName, NumSwapPages*PageSize);

  if ((swapFile = fileSystem->Open(SwapFileName)) == NULL) {
    ASSERT (false);
  }
}

SwapManager::~SwapManager() {
  //
  // Do nothing.
  //
  unlink (SwapFileName);
}

int SwapManager::get_next_free_frame() {
  //
  // Find the first frame flag that == 0, and set it to 1. 
  //
  int free_frame = frame_flags->Find();

  //
  // If there was no clear frame flag, set nerrno to reflect the fact that
  // we're out of memory.
  //
  if (free_frame == -1) {
    return -ENOMEM;
  }

  //
  // Return the number of the free frame, or -1 if there wasn't one.
  //
  return free_frame;
}

void SwapManager::release_frame(size_t offset, AddrSpace *addrspace) {
  int frame_num = (int) (offset / PageSize);

  //  ASSERT (addrspace == currentThread->space);

  //
  // Clear the page flag.
  //
  if (Frames[frame_num].numOwners == 1) {
    frame_flags->Clear(frame_num);
    Frames[frame_num].owners = NULL;
    Frames[frame_num].numOwners = 0;
  } else {
    AddrSpace **newOwners;
    int j = 0;
    
    newOwners = new AddrSpace *[Frames[frame_num].numOwners - 1];
    for (int i = 0; i < Frames[frame_num].numOwners; i++) {
      if (Frames[frame_num].owners[i] != addrspace) {
	newOwners[j] = Frames[frame_num].owners[i];
	j++;
      }
    }
    delete [] Frames[frame_num].owners;
    
    Frames[frame_num].owners = newOwners;
    Frames[frame_num].numOwners--;
  }
}


Frame * SwapManager::get_frame( size_t offset ) {
  int frame_num = (int) (offset / PageSize);

  if ( ( frame_num > NumSwapPages )||( frame_num < 0 ) ) {
    return NULL;
  }
  return ( &Frames[frame_num] );
}


int SwapManager::add_frame_owner( size_t offset, Thread * thread, 
				  int owner_page ) {
  AddrSpace **newOwners;
  int frame_number = offset / PageSize;

  if ( ( frame_number > NumSwapPages )||( frame_number < 0 ) ) {
    return -1;
  }
  
  newOwners = new AddrSpace *[Frames[frame_number].numOwners + 1];
  for (int i = 0; i < Frames[frame_number].numOwners; i++) {
    newOwners[i] = Frames[frame_number].owners[i];
  }
  if (Frames[frame_number].numOwners) {
    delete [] Frames[frame_number].owners;
  }

  Frames[frame_number].owners = newOwners;
  Frames[frame_number].owners[Frames[frame_number].numOwners] = thread->space;

  if (Frames[frame_number].numOwners) {
    ASSERT (Frames[frame_number].owners_page_number == owner_page);
  }

  Frames[frame_number].owners_page_number = owner_page;
  Frames[frame_number].numOwners++;

  return 0;
}




#endif

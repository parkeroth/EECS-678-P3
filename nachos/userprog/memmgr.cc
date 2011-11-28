// $Id: memmgr.cc,v 1.7 2009/09/26 20:24:35 mjantz Exp $
//
// This is the implementation of the Memory Manager object.
//
// Authors: Matt Peters, Doug McClendon, Shyam Pather

#ifdef USER_PROGRAM

#pragma implementation "userprog/memmgr.h"

#include "memmgr.h"
#include "nerrno.h"
#include "system.h"
#include <machine.h>

MemoryManager::MemoryManager() {
  //
  // Clear all the page flags. This means that all pages are free.
  //
  page_flags = new BitMap(NumPhysPages);
  for (int i = 0; i < NumPhysPages; i++) {
    page_flags->Clear(i);
    Frames[i].owners = NULL;
    Frames[i].numOwners = 0;
  }
}

MemoryManager::~MemoryManager() {
  //
  // Do nothing.
  //
}

int MemoryManager::get_next_free_page() {

  // To check if the current process owns more pages than allowed by it's 
  // working set size 
  if (currentThread->space->TooManyFrames()) {
    return -1;
  }

  //
  // Find the first page flag that == 0, and set it to 1. 
  //
  int free_page = page_flags->Find();

  //
  // If there was no clear page flag, set nerrno to reflect the fact that
  // we're out of memory.
  //
  if (free_page == -1) {
    return -ENOMEM;
  }

  //
  // Return the number of the free page, or -1 if there wasn't one.
  //
  return free_page;
}

void MemoryManager::release_page(int page_num, AddrSpace *addrspace) {

  //  ASSERT (addrspace == currentThread->space);

  //
  // Clear the page flag.
  //
  if (Frames[page_num].numOwners == 1) {
    page_flags->Clear(page_num);
    Frames[page_num].owners = NULL;
    Frames[page_num].numOwners = 0;
  } else {
    AddrSpace **newOwners;
    int j = 0;

    newOwners = new AddrSpace *[Frames[page_num].numOwners - 1];
    for (int i = 0; i < Frames[page_num].numOwners; i++) {
      if (Frames[page_num].owners[i] != addrspace) {
	newOwners[j] = Frames[page_num].owners[i];
	j++;
      }
    }
    delete [] (Frames[page_num].owners);
    
    Frames[page_num].owners = newOwners;
    Frames[page_num].numOwners--;

  }
  
}


// VIRTUAL_MEMORY

// MemoryManager::get_frame
// 
// Get the frame record corresponding to a particular frame.
//
// Argument:
// number  : Frame number
//
// Return value:
// Pointer to the frame record for the frame, if the number passed in was 
// valid, or NULL otherwise.

Frame * MemoryManager::get_frame( int number ) {
  if ( ( number > NumPhysPages )||( number < 0 ) ) {
    return NULL;
  }
  return ( &Frames[number] );
}


// MemoryManager::add_frame_owner
//
// Add a new owner to the list of owners of a frame.
//
// Arguments:
// frame_number : The number of the frame to which we are going to add an owner
// thread       : Thread to add to the list of owners
// owner_page   : Virtual page number of the frame in the thread's page table.

int MemoryManager::add_frame_owner( int frame_number, Thread * thread, 
				    int owner_page ) {

  // The basic strategy here is to replace the list of owner with a new list,
  // where the new one includes the thread we want to add.
 
  AddrSpace **newOwners;

  if ( ( frame_number > NumPhysPages )||( frame_number < 0 ) ) {
    return -1;
  }

  // Make sure the thread is not an owner already
  for (int i = 0; i < Frames[frame_number].numOwners; i++)
    {
      ASSERT (thread->space != Frames[frame_number].owners[i]);
    }

  // Allocate an array that is 1 bigger than the old one.
  newOwners = new AddrSpace *[Frames[frame_number].numOwners + 1];
  if (newOwners == NULL) {
    return -ENOMEM;
  }
  
  // Copy all the old owners
  for (int i = 0; i < Frames[frame_number].numOwners; i++) {
    newOwners[i] = Frames[frame_number].owners[i];
  }

  // Delete the old list
  if (Frames[frame_number].numOwners) {
    delete [] (Frames[frame_number].owners);
  }

  // Assign the new list to the frame record, and add our thread to it.
  Frames[frame_number].owners = newOwners;
  Frames[frame_number].owners[Frames[frame_number].numOwners] = thread->space;

  if (Frames[frame_number].numOwners) {
    // ASSERT (Frames[frame_number].owners_page_number == owner_page);
    if (Frames[frame_number].owners_page_number != owner_page)
      {
	register int i;
	printf ("Frame: %d; owner_page = %d, but owners_page_number = %d\n",
		frame_number, owner_page,
		Frames[frame_number].owners_page_number);
	for (i = 0; i < Frames[frame_number].numOwners; i++)
	  {
	    printf ("Owner %d: Thread %s\n", i,
		    Frames[frame_number].owners[i]->owner->GetName());
	  }
	printf ("Adding thread: %s\n", thread->GetName());
	ASSERT(false);
      }
  }

  Frames[frame_number].owners_page_number = owner_page;

  // Increment the number of owners of the page.
  Frames[frame_number].numOwners++;

  return 0;
}


// MemoryManager::pagein
//
// Bring a page into main memory.
//
// Arguments:
// page_number : Virtual page # to bring in.
// addrspace   : Pointer to thread's address space object (containing its
//               page table).

void MemoryManager::pagein( int page_number, AddrSpace * addrspace ) {
  TranslationEntry* local;
  int dest_frame;
  OpenFile *swapfile = swap->file();

  //
  // Get the translation entry (from the thread's page table) of the page  
  // that we are going to bring in.
  //
  local = addrspace->get_page_ptr( page_number );
  
  //
  // Try to find a free memory frame for it.
  //
  if ( ( dest_frame = get_next_free_page() ) < 0 ) {
    // 
    // There was no free memory frame, so we are going to choose a victim
    // page, swap it out, and then use it's frame to bring in the page we
    // want. 
    //
    dest_frame = Choose_Victim(-1);
    pageout( dest_frame );
  }

  //
  // If the page is supposed to be filled with zeroes, then we do that, 
  // otherwise we read it from the file (this will either be the original
  // executable or the swap file.
  //
  if (!local->zero) {
    local->File->ReadAt( &(machine->mainMemory[ dest_frame * PageSize ]), 
			 PageSize, local->offset );
  } else {    
    memset (&(machine->mainMemory [dest_frame * PageSize]), 0, PageSize);
  }

  //
  // Initialize the frame record for the memory frame. This is done 
  // differently, depending on whether we read the page in from a swapfile, 
  // or from the original executable.
  //
  if (local->File == swapfile) {
    //
    // Read the frame record from the swapfile.
    //
    Frame *frame = swap->get_frame (local->offset);

    //
    // Copy the information from the frame record in the swapfile to that of
    // main memory (for the frame we are using). This information includes
    // the owners of the page, the owner's page number, and the number of 
    // owners.
    //
    if ( Frames[dest_frame].owners != NULL ) {
      delete [] ( Frames[dest_frame].owners ); 
    }

    Frames[ dest_frame ].owners = new AddrSpace * [ frame->numOwners ];
    for ( int i = 0; i < frame->numOwners ; i ++ ) {
      Frames[ dest_frame ].owners[i] = frame->owners[i];
    }

    Frames[ dest_frame ].owners_page_number = frame->owners_page_number;
    Frames[ dest_frame ].numOwners = frame->numOwners;

  } else {

    // Set up the frame record for the memory frame. This includes setting 
    // up the owners, number of owners, owner's page # etc. 
    Frames[ dest_frame ].owners = new AddrSpace * [1];
    Frames[ dest_frame ].owners[0] = addrspace;
    Frames[ dest_frame ].owners_page_number = page_number;
    Frames[ dest_frame ].numOwners = 1;
    // We paged in from disk, clear COW so this will be our own copy
    local->cow = false;
  }

  // Change the translation entries in the page tables of each of the 
  // owners so that they know that the page is in main mem now. 
  for (int i = 0; i < Frames[ dest_frame ].numOwners; i++) {
    TranslationEntry *te = Frames[ dest_frame ].owners[i]->
      get_page_ptr (page_number);
    te->valid = true;
    te->use = false;
    te->dirty = false;
    te->physicalPage = dest_frame;
    te->clearSC ();
    te->setTime (stats->totalTicks);
  }

  stats->numPageIns++;
  if (currentThread) {
      currentThread->procStats->numPageIns++;
  }
}


// MemoryManager::pageout
//
// Swap a page out of main memory.
//
// Arguments:
// victim   : Physical page number of the page to be swapped out.

void MemoryManager::pageout( int victim ) {
  OpenFile * swapfile = swap->file();
  int swap_num;
  TranslationEntry *local;

  ASSERT (Frames[ victim ].owners != NULL);
  ASSERT (Frames[ victim ].owners[0] != NULL);
  // 
  // Get the translation entry for the physical page from the page table 
  // of one of the owners. We use the first owner here, but it should not
  // matter which one we use (since each owner should have exactly the same
  // translation entry for the page).
  //
  local = Frames[ victim ].owners[0]->
    get_page_ptr ( Frames[ victim ].owners_page_number);

  // 
  // Check if the page is dirty. If so, we need to send it to the swapfile. 
  // Otherwise, we can just read it from the original executable when we 
  // need it, so we don't need to put it in the swapfile.
  //
  if (local->dirty)  // It is dirty.
    {
      if (local->File != swapfile) //never been swapped out
	{
	  // 
	  // Get the next free frame in the swap file. If there isn't one,
	  // we're in pretty bad shape, so we just assert to get out.
	  //
	  swap_num = swap->get_next_free_frame(); 
	  ASSERT (swap_num >= 0);

	  // 
	  // Go through the list of owners, and change the translation entries
	  // for the page being swapped out in their page tables, so that they
	  // are aware that the page has been swapped out when they try to
	  // reference it later.
	  //
	  for (int i = 0; i < Frames[ victim ].numOwners; i++)
	    {
	      TranslationEntry *te = Frames[ victim ].owners[i]->
		get_page_ptr ( Frames[ victim ].owners_page_number);
	      te->File = swapfile;
	      te->offset = swap_num * PageSize;
	      te->zero = false;
	    }
	}

      swapfile->WriteAt (&(machine->mainMemory[victim * PageSize]), PageSize,
			 local->offset);

      stats->numPageOuts++;
      for (int i = 0; i < Frames[ victim ].numOwners; i++)
	{
	  Frames[victim].owners[i]->owner->procStats->numPageOuts++;
	}
    }
  else
    {
      // If we don't need to swap out, and we'll swap back in from something
      // other than the swapfile, clear the COW so that each process will
      // swap it's own copy of the page back in when necessary
      if (local->File != swapfile)
	{
	  for (int i = 0; i < Frames[ victim ].numOwners; i++)
	    {
	      TranslationEntry *te = Frames[ victim ].owners[i]->
		get_page_ptr ( Frames[ victim ].owners_page_number);
	      te->cow = false;
	    }
	}
    }

  if (local->File == swapfile)
    {
      // 
      // Set the frame record in the swapfile.
      //
      Frame *frame = swap->get_frame (local->offset);
      if (frame->owners != NULL)
	{
	  delete [] frame->owners; 
	}
      frame->owners = new AddrSpace *[ Frames[victim].numOwners ];
      for (int i = 0; i < Frames[victim].numOwners ; i++)
	{
	  frame->owners[i] = Frames[victim].owners[i];
	}

      frame->owners_page_number = Frames[ victim ].owners_page_number;
      frame->numOwners =  Frames[ victim ].numOwners;
    }

  // 
  // For each of the owners, set the valid bit of the page table entries 
  // for the page to false.
  //
  for (int i = 0; i < Frames[ victim ].numOwners; i++)
    {
      TranslationEntry *te = Frames[ victim ].owners[i]->
	get_page_ptr ( Frames[ victim ].owners_page_number);
      te->valid = false;
    }

  // Clear out old information
  delete [] (Frames[victim].owners);
  Frames[victim].owners = NULL;
  Frames[victim].numOwners = 0;
}

// MemoryManager::Choose_Victim
// 
// Choose which memory frame to swap out. This is where the heart of the page 
// replacement policy gets implemented.
//
// Arguments:
// notMe    : The number of a page that you do not want swapped out.

int MemoryManager::Choose_Victim (int notMe) {
  int page = -1;
  
  if ((pageReplPolicy == DUMB) || (!currentThread->space->TooManyFrames())) {
    page = Dumb_Choose_Victim (notMe);
  } else if (pageReplPolicy == FIFO) {
    page = currentThread->space->FIFO_Choose_Victim (notMe);
  } else if (pageReplPolicy == LRU) {
    page = currentThread->space->LRU_Choose_Victim (notMe);
  } else if (pageReplPolicy == SECONDCHANCE) {
    page = currentThread->space->SC_Choose_Victim (notMe);
  } else {
    printf ("Unknown page replacement algorithm.\n");
    ASSERT (false);
  }

  ASSERT (page != -1);
  ASSERT (page < NumPhysPages);
  return page;
}

int MemoryManager::Dumb_Choose_Victim (int notMe) {
  static int i = 87;

  do {
    i += 17;
    i %= NumPhysPages;
  }
  while ((i == notMe) || (Frames[i].owners == NULL));

  return i;
}
#endif

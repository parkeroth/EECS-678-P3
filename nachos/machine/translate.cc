// translate.cc 
//	Routines to translate virtual addresses to physical addresses.
//	Software sets up a table of legal translations.  We look up
//	in the table on every memory reference to find the true physical
//	memory location.
//
// Two types of translation are supported here.
//
//	Linear page table -- the virtual page # is used as an index
//	into the table, to find the physical page #.
//
//	Translation lookaside buffer -- associative lookup in the table
//	to find an entry with the same virtual page #.  If found,
//	this entry is used for the translation.
//	If not, it traps to software with an exception. 
//
//	In practice, the TLB is much smaller than the amount of physical
//	memory (16 entries is common on a machine that has 1000's of
//	pages).  Thus, there must also be a backup translation scheme
//	(such as page tables), but the hardware doesn't need to know
//	anything at all about that.
//
//	Note that the contents of the TLB are specific to an address space.
//	If the address space changes, so does the contents of the TLB!
//
// DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#pragma implementation "machine/translate.h"

#include "copyright.h"
#include "machine.h"
#include "addrspace.h"
#include "system.h"

//----------------------------------------------------------------------
// Machine::ReadMem
//      Read "size" (1, 2, or 4) bytes of virtual memory at "addr" into 
//	the location pointed to by "value".
//
//   	Returns false if the translation step from virtual to physical memory
//   	failed.
//
//	"addr" -- the virtual address to read from
//	"size" -- the number of bytes to read (1, 2, or 4)
//	"value" -- the place to write the result
//----------------------------------------------------------------------

bool
Machine::ReadMem(int addr, int size, void *value)
{
    int data;
    ExceptionType exception;
    int physicalAddress;
    
    DEBUG( (char *)DB_ADDRESS , (char *)"Reading VA 0x%x, size %d\n", addr, size);
    
#ifdef REMOTE_USER_PROGRAM_DEBUGGING
    int breakAddr;
    if (breakpoints.queryBreakpoint(addr, size, ReadWp, breakAddr))
	BREAKPOINT(breakAddr);
#endif

    exception = Translate(addr, &physicalAddress, size, false);
    if (exception != NoException) {
	machine->RaiseException(exception, addr);
	return false;
    }
    switch (size) {
      case 1:
	data = machine->mainMemory[physicalAddress];
	*(char *)value = (char) data;
	break;
	
      case 2:
	data = *(unsigned short *) &machine->mainMemory[physicalAddress];
	*(unsigned short *)value = ShortToHost(data);
	break;
	
      case 4:
	data = *(unsigned int *) &machine->mainMemory[physicalAddress];
	*(unsigned int *)value = WordToHost(data);
	break;
        // FIXME: We need a message about what went wrong
      default:
        ASSERT(false);

    }
    
    // DEBUG( DB_ADDRESS , "\tvalue read = %8.8x\n", *value);
    return true;
}

//----------------------------------------------------------------------
// Machine::WriteMem
//      Write "size" (1, 2, or 4) bytes of the contents of "value" into
//	virtual memory at location "addr".
//
//   	Returns false if the translation step from virtual to physical memory
//   	failed.
//
//	"addr" -- the virtual address to write to
//	"size" -- the number of bytes to be written (1, 2, or 4)
//	"value" -- the data to be written
//----------------------------------------------------------------------

bool
Machine::WriteMem(int addr, int size, int value)
{
    ExceptionType exception;
    int physicalAddress;
     
    DEBUG( (char *)DB_ADDRESS , (char *)"Writing VA 0x%x, size %d, value 0x%x\n", addr, size, value);

#ifdef REMOTE_USER_PROGRAM_DEBUGGING
    int breakAddr;
    if (breakpoints.queryBreakpoint(addr, size, WriteWp, breakAddr))
	BREAKPOINT(breakAddr);
#endif

    exception = Translate(addr, &physicalAddress, size, true);
    if (exception != NoException) {
	machine->RaiseException(exception, addr);
	return false;
    }
    switch (size) {
      case 1:
	machine->mainMemory[physicalAddress] = (unsigned char) (value & 0xff);
	break;

      case 2:
	*(unsigned short *) &machine->mainMemory[physicalAddress]
		= ShortToMachine((unsigned short) (value & 0xffff));
	break;
      
      case 4:
	*(unsigned int *) &machine->mainMemory[physicalAddress]
		= WordToMachine((unsigned int) value);
	break;
        // FIXME: We need a message about what went wrong
      default:
        ASSERT(false);
}
    return true;
}

//----------------------------------------------------------------------
// Machine::Translate
// 	Translate a virtual address into a physical address, using 
//	either a page table or a TLB.  Check for alignment and all sorts 
//	of other errors, and if everything is ok, set the use/dirty bits in 
//	the translation table entry, and store the translated physical 
//	address in "physAddr".  If there was an error, returns the type
//	of the exception.
//
//	"virtAddr" -- the virtual address to translate
//	"physAddr" -- the place to store the physical address
//	"size" -- the amount of memory being read or written
// 	"writing" -- if true, check the "read-only" bit in the TLB
//----------------------------------------------------------------------

ExceptionType
Machine::Translate(int virtAddr, int* physAddr, int size, bool writing)
{
    unsigned int vpn, offset;
    TranslationEntry *entry;
    unsigned int pageFrame;

    DEBUG( (char *)DB_ADDRESS , (char *)"\tTranslate 0x%x, %s: ", virtAddr, writing ? "write" : "read");

    //FIXME: The name of this method should be TranslateAndCheck or this check
    //and the one at the end (see the next FIXME) should be moved elsewhere.
// check for alignment errors
// if any are present we report it and return the address at which it occured.
    if (((size == 4) && (virtAddr & 0x3)) || ((size == 2) && (virtAddr & 0x1))){
	DEBUG( (char *)DB_ADDRESS , (char *)"alignment problem at %d, size %d!\n", virtAddr, size);
	return AddressErrorException;
    }
    
    // The MIPS hardware does not know how to walk the page table, so the TLB
    // is filled from within an exception handler.  Therefore,
    // we must have either a TLB or a page table, but not both, because we
    // should either simulate the real hardware and throw an exception on a
    // miss which would invoke separate code to replace TLB entries, etc., or
    // just use the page table directly, simulating a machine where the TLB is
    // always sufficient.
    ASSERT(tlb == NULL || pageTable == NULL);	
    ASSERT(tlb != NULL || pageTable != NULL);	

// calculate the virtual page number, and offset within the page,
// from the virtual address
    vpn = (unsigned) virtAddr / PageSize;
    offset = (unsigned) virtAddr % PageSize;
    
    if (tlb == NULL) {		// => page table => vpn is index into table
	/* Check the requested VPN for sanity */
	if (vpn >= pageTableSize) {
	    DEBUG( (char *)DB_ADDRESS , (char *)"virtual page # %d too large for page table size %d!\n", 
			vpn, pageTableSize);
	    return AddressErrorException;
	} else if (!pageTable[vpn].valid) {
	    DEBUG( (char *)DB_ADDRESS , (char *)"page fault on page # %d\n", vpn);
	    WriteRegister (BadVAddrReg, vpn);
	    return PageFaultException;
	}
	entry = &pageTable[vpn];
    } else {
	int i = 0;
        for (entry = NULL; i < TLBSize; i++)
    	    if (tlb[i].valid && (tlb[i].virtualPage == vpn)) {
		entry = &tlb[i];			// FOUND!
		break;
	    }
	if (entry == NULL) {				// not found
    	    DEBUG( (char *)DB_ADDRESS , (char *)"*** no valid TLB entry found for this virtual page!\n");
	    WriteRegister (BadVAddrReg, vpn);
    	    return PageFaultException;		// really, this is a TLB fault,
						// the page may be in memory,
						// but not in the TLB
	}

	if (entry->readOnly && writing) {	// writing to a read-only page
	  DEBUG( (char *)DB_ADDRESS , (char *)"%d mapped read-only at %d in TLB!\n", virtAddr, i);
	  return ReadOnlyException;
	}
    }

    // VIRTUAL_MEMORY
    if (entry->cow && writing) {
      DEBUG( (char *)DB_ADDRESS , (char *)"Copy-on-write hit at %d\n", virtAddr);
      currentThread->space->duplicatePage (vpn);
      entry = &pageTable[vpn];
    }

    pageFrame = entry->physicalPage;

    // if the pageFrame is too big, there is something really wrong! 
    // An invalid translation was loaded into the page table or TLB. 
    if (pageFrame >= NumPhysPages) { 
	DEBUG( (char *)DB_ADDRESS , (char *)"*** frame %d > %d!\n", pageFrame, NumPhysPages);
	return BusErrorException;
    }
    entry->use = true;		// set the use, dirty bits


    if (writing)
	entry->dirty = true;
    *physAddr = pageFrame * PageSize + offset;
    // FIXME: This should not be an assert; it should raise some exception
    ASSERT((*physAddr >= 0) && ((*physAddr + size) <= MemorySize));
    DEBUG( (char *)DB_ADDRESS , (char *)"phys addr = 0x%x\n", *physAddr);
    return NoException;
}


// FIXME: Wrap these in ASSIGNMENT markers

// -----------------------------------------------------------------------
// clearSC
// Purpose: Clears the second chance information for this page
//
// NOTE: This function is used throughout the memory management code
// -----------------------------------------------------------------------
void TranslationEntry::clearSC () {
  // reset reference bit
  use = false;
}


// -----------------------------------------------------------------------
// clearRefHistory
// Purpose: Clears the reference history for this page
//
// NOTE: This function is used throughout the memory management code
// -----------------------------------------------------------------------
void TranslationEntry::clearRefHistory () {
}


// -----------------------------------------------------------------------
// setTime
// Purpose: Sets the load time for this page
//
// NOTE: This function is used throughout the memory management code
// -----------------------------------------------------------------------
void TranslationEntry::setTime (unsigned int theTime) {
}


// -----------------------------------------------------------------------
// getTime
// Purpose: Sets the load time for this page
//
// NOTE: This function is used throughout the memory management code
// -----------------------------------------------------------------------
unsigned int TranslationEntry::getTime () {
    // return the access time here
    return 0;
}


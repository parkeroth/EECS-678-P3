// -*- C++ -*-
// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we do not keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#pragma interface "userprog/addrspace.h"

#include "copyright.h"
#include "filesys.h"
#include "openfile.h"
#include "thread.h"
#include "system.h"
#include "elf.h"

class Thread;

#define UserStackSize		4096 	// increase this as necessary!

// Section types
#define REGINFO 0
#define TEXT    1
#define RDATA   2
#define DATA    3
#define BSS     4
#define SBSS    5
#define NUM_SECTIONS 6

typedef struct nachosHeader
{
  Elf32_Elf_header file_header;
  Elf32_Section_header section[NUM_SECTIONS];
} NachosHeader;

class AddrSpace {
public:
  const Thread *owner;

  AddrSpace (Thread *t) :
    owner(t),
    pageTable(NULL), numPages(0), execFile(NULL) {}
  ~AddrSpace();			// Deallocate an address space

  int InitSpace(int numpages);
  int InitSpace(OpenFile *executable);
  int ModifySpace(OpenFile *executable);
  void InitRegisters(void);		// Initialize user-level CPU registers,
  // before jumping to user code
  void SaveState(void);			// Save/restore address space-specific
  void RestoreState(void);		// info on a context switch 

  TranslationEntry *get_page_ptr (unsigned int virtPage) const {
    return (virtPage >= numPages) ? NULL : &(pageTable[virtPage]);
  }

  int CopyFrom(Thread *ourThread);
  void duplicatePage (int page_number);

  int FIFO_Choose_Victim (int notMe);
  int LRU_Choose_Victim (int notMe);
  int SC_Choose_Victim (int notMe);
  int TooManyFrames(void);


private:
  void CopyPageTable (TranslationEntry *oldPT, TranslationEntry *newPT, 
		      int numpages);
  int SetupTable(void);
  int Setup_Load(OpenFile *executable, NachosHeader *nachosH);
  unsigned int NumPhysPagesOwned (void);

  TranslationEntry *pageTable;	      // Assume linear page table translation
                                      // for now!
  unsigned int numPages;	      // Number of pages in the virtual 
                                      // address space
  OpenFile *execFile;                 // Executable that is currently running
                                      // in this address space.  NULL if none
  unsigned int startAddress;          // Where to start executing
};

#endif // ADDRSPACE_H

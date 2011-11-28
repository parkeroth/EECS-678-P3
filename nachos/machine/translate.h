// translate.h 
//	Data structures for managing the translation from 
//	virtual page # -> physical page #, used for managing
//	physical memory on behalf of user programs.
//
//	The data structures in this file are "dual-use" - they
//	serve both as a page table entry, and as an entry in
//	a software-managed translation lookaside buffer (TLB).
//	Either way, each entry is of the form:
//	<virtual page #, physical page #>.
//
// DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef TRANSLATE_H
#define TRANSLATE_H

#pragma interface "machine/translate.h"

#include "copyright.h"
#include "utility.h"
#include "openfile.h"

// The following class defines an entry in a translation table -- either
// in a page table or a TLB.  Each entry defines a mapping from one 
// virtual page to one physical page.
// In addition, there are some extra bits for access control (valid and 
// read-only) and some bits for usage information (use and dirty).

class TranslationEntry {
  public:
    unsigned char history;       // One byte for history bits
    unsigned int load_time;      // Time stamp counter

    unsigned int virtualPage;  	// The page number in virtual memory.
    unsigned int physicalPage;  // The page number in real memory (relative
                                // to the start of "mainMemory"

    bool valid;         // Unless this bit is set, the translation is
			// ignored because the virtual page is not
			// currently mapped to a physical page.
    bool readOnly;	// If this bit is set, the user program is not allowed
			// to modify the contents of the page.
    bool use;           // This bit is set by the hardware every time the
			// page is referenced or modified.
    bool dirty;         // This bit is set by the hardware every time the
			// page is modified.
  // VIRTUAL_MEMORY
    OpenFile* File;     // This is the file where the page can be found
    size_t offset;      // this is the offset within the file 
                        //    where the page starts
    bool zero;          // do we zero out the page ( for uninit/stack ) data

    bool cow;           // copy-on-write; do we need to copy the page
			// before we write to it


    void clearSC ();
    void clearRefHistory ();
    void setTime (unsigned int theTime);
    unsigned int getTime ();
};

#endif /* TRANSLATE_H */

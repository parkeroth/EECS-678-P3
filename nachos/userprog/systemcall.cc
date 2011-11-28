// =======================================================
// File: systemcall.cc
// Purpose: This file contains the function do_system_call()
//          which cases on the type of system call and then
//          calls the appropriate system function.  This file
//          also contains those system functions ( i.e. System_Fork...)
// =======================================================

#include "system.h"
#include "systemcall.h"
#include "thread.h"
#include "nerrno.h"
#include "defs.h"
#include "filesys.h"
#include "fdt.h"
#include "console.h"
#include <stdio.h>
#include <unistd.h>
/*
 * DSUI_REMOVAL_LOCATIONS
 * The code enclosed in **all** the F_DSUI ifdefs in this file was 
 * causing problems for pa2-1 during compilation, so I put the ifdefs back in.
 */
#include "nachos_dsui.h"

#ifdef USER_PROGRAM

void do_system_call(int syscall_num) {
  int reg4, reg5, reg6, reg7, returnvalue;
  // these are the argument registers used by the system call
  // functions.
  reg4 = machine->ReadRegister(4);
  reg5 = machine->ReadRegister(5);
  reg6 = machine->ReadRegister(6);
  reg7 = machine->ReadRegister(7);
  
  switch (syscall_num) {
  case SC_Halt:
    System_Halt();
    break;
  case SC_Exit:
    System_Exit((int)reg4);
    break;
  case SC_Exec:
    returnvalue = System_Exec ((char *) reg4);
    break;
  case SC_Wait:
    returnvalue = System_Wait ((int *) reg4);
    break;
  case SC_Create:
    returnvalue = System_Create ((char *) reg4);
    break;
  case SC_Open:
    returnvalue = System_Open ((char *) reg4);
    break;
  case SC_Read:
    returnvalue = System_Read ((int) reg4, (char*) reg5, (int) reg6);
    break;
  case SC_Write:    
    returnvalue = System_Write ((int) reg4, (char *) reg5, (int) reg6);
    break;
  case SC_Close:    
    returnvalue = System_Close ((int) reg4);
    break;
  case SC_Unlink:
    returnvalue = System_Unlink ((char *) reg4);
    break;
  case SC_Fork:     
    returnvalue = System_Fork();
    break;
  case SC_GetPID:
    returnvalue = System_GetPID ();
    break;
  case SC_GetPPID:
    returnvalue = System_GetPPID ();
    break;
  case SC_Yield:    
    System_Yield();
    break;
  case SC_Nice:
    // reg 4 is where the Parameter value is stored
    System_Nice ((int) reg4);
    break;
  case SC_Echo:
    returnvalue = System_Echo ((char *) reg4, (int) reg5);
    break;
  case SC_NameThread:
    returnvalue = System_NameThread((char *) reg4);
    break;

/* ---------------------------------------------------------- */
  case SC_NachosUserEvent:
  	System_NachosUserEvent((char *) reg4, (char *) reg5, (int) reg6);
	break;
/* ---------------------------------------------------------- */

  default:
    fprintf (stderr, "Nonexistent system call: %d\n", syscall_num);
    returnvalue = -1;
  };
  machine->WriteRegister(2, returnvalue);
}

//
//
int System_Create (char *user_space_filename) {
  char * filename = new char[MAXFILENAMELENGTH];

  if (!filename) { 
    return -ENOMEM;
  }
  copy_from_user (user_space_filename, filename);

  if (!fileSystem->Create (filename, 0)) {
    return -ENOENT;
  }

  delete [] filename;

  return 0;
}

//
//
int System_Open (char *user_space_filename) {
  int fd;
  char * filename = new char[MAXFILENAMELENGTH];
  OpenFile *file;
  
  if (!filename) { 
    return -ENOMEM;
  }
  copy_from_user (user_space_filename, filename);

  if ((fd = currentThread->find_next_available_fd ()) < 0) {
    delete [] filename;
    return -EMFILE;
  }

  file = fileSystem->Open(filename);
  if (file == NULL) {
    delete [] filename;
    return -ENOENT;
  }
  FDTEntry *fdte = new FDTEntry;
  if (!fdte) {
    delete [] filename;
    return -ENOMEM;
  }
  fdte->type = DiskFile;
  fdte->openfile = file;
  currentThread->setFD (fd, fdte);

  delete [] filename;
  return fd;
}

// ================================================================
// System_Read:
// Paramters: register 6 contains the # of bytes to write.
//            Register 5 contains a pointer to the string read.
//            Register 4 contains the file descriptor to read from.
// Returns:# of bytes read is placed in register 2.
// This is the read system call.  It is called from do_system_call
// ================================================================
int System_Read (int from_fd, char* to_user_space, int num_to_read) {
   char *buffer;
   int bytesread;
   OpenFile *file;
   
   buffer = new char[num_to_read];
   if (buffer == NULL) {
     return -ENOMEM;
   }
   
   FDTEntry *fdte = currentThread->getFD (from_fd);
   if (!fdte) {
     delete [] buffer;
     return -EBADF;
   }
   
   switch (fdte->type) {
   case ConsoleFile :
     bytesread = ConsoleRead (buffer, num_to_read);
     break;
   case DiskFile :
     file = fdte->openfile;
     bytesread = file->Read (buffer, num_to_read);
     break;
   default :
     delete [] buffer;
     return -EBADF;
     break;
   }
   
   for (ssize_t i = 0; i < bytesread; i++) {
     bool ok;
     ok = machine->WriteMem ((long)(to_user_space + i), 1, (long) buffer[i]);
     // If the write failed (probably due to a page fault), try again.
     // If it still fails, abort the write and return the number read
     if (!ok) {
       ok = machine->WriteMem ((long)(to_user_space + i), 1, (long) buffer[i]);
       if (!ok) {
	 delete [] buffer;
	 return (int)i;
       }
     }
   }
   
   delete [] buffer;
   return (int)bytesread;
}


// ================================================================
// System_Write:
// Parameters: register 6 contains the # of bytes to write.  Register
//             5 contains a pointer to the string to write.
//             Register 4 contains the file descriptor to write to.
// Returns: # of bytes written in register 2.
// This is the write system call.  It is called from do_system_call.
// ================================================================
int System_Write (int to_fd, char * from_user_space, int num_to_write) {
  int byteswritten;
  char* buffer;
  OpenFile *file;
  
  buffer = new char[num_to_write + 1];
  
  if (buffer == NULL) {
    return -ENOMEM;
  }

  FDTEntry *fdte = currentThread->getFD (to_fd);
  if (!fdte) {
    delete [] buffer;
    return -EBADF;
  }
  
  for (int i = 0; i < num_to_write; i++) {
    bool ok;
    ok = machine->ReadMem ((long)(from_user_space + i), 1, (long *)&buffer[i]);
    // If the read failed (probably due to a page fault), try again.
    // If it still fails, abort the read and write what was available
    if (!ok) {
      ok = machine->ReadMem((long)(from_user_space + i), 1, (long *)&buffer[i]);
      if (!ok) {
	num_to_write = i;
	break;
      }
    }
  }
  buffer [num_to_write] = '\0';
  
  
  switch (fdte->type) {
  case ConsoleFile :
    byteswritten = ConsoleWrite (buffer, num_to_write);
    break;
  case DiskFile :
    file = fdte->openfile;
    byteswritten = file->Write (buffer, num_to_write);
    break;
  default :
    delete [] buffer;
    return -EBADF;
    break;
  }
  
  delete [] buffer;
  return byteswritten;
}


//
//
int System_Close (int fd) {
  OpenFile *file;

  FDTEntry *fdte = currentThread->getFD (fd);
  if (!fdte) {
    return -EBADF;
  }

  switch (fdte->type) {
  case ConsoleFile :
    break;
  case DiskFile :
    file = fdte->openfile;
    break;
  default :
    return -EBADF;
    break;
  }
  currentThread->setFD (fd, (FDTEntry *) NULL);

  return 0;  
}


//
//
int System_Unlink (char *user_space_filename) {
  char * filename = new char[MAXFILENAMELENGTH];

  if (!filename) { 
    return -ENOMEM;
  }
  copy_from_user (user_space_filename, filename);

  if (!fileSystem->Remove (filename)) {
    delete [] filename;
    return -ENOENT;
  }

  delete [] filename;

  return 0;
}


// ================================================================
// System_Halt:
// ================================================================
void System_Halt() {
  interrupt->Halt();
}


// ================================================================
// System_Exit:
// ================================================================
void System_Exit (int exitvalue) {
  Thread *parent;

  DSTRM_EVENT(SYSCALL, EXIT, currentThread->Get_Id());

  // update the PCB to reflect that the thread has invoked the exit
  // systemcall
  currentThread->ReachedExit();

  while (currentThread->Get_Num_Children () > 0) {
    System_Wait (0);
  }

  if (printProcStats && currentThread->procStats) {
    currentThread->procStats->ShortPrint (currentThread->Get_Id());
  }
  
  parent = currentThread->Get_Parent_Ptr ();
  if (!parent) {
    currentThread->Finish ();
  }
  currentThread->thread_exit_status = false;
  currentThread->Set_Exit_Val (exitvalue);

  parent->Queue_Child (currentThread);

  parent->ChildExited->V ();

  if (currentThread->thread_exit_status)
    {
      currentThread->Finish ();
    }
  else
    {
      currentThread->Sleep (ZOMBIE);
    }
}


// ===================================================================
// System_Wait :
// ===================================================================
int System_Wait (int *exitvalue) {
  Thread *child;
  int childexitvalue;
  int pid;
  
  if (currentThread->Get_Num_Children () == 0) {
    return -ECHILD;
  }
  
  // currentThread waiting on the semaphore ChildExited
  currentThread->ChildExited->P ();

  child = currentThread->UnQueue_Child ();

  if (!child) {
    ASSERT (false);
  }
  
  if (exitvalue) {
    bool ok;

    childexitvalue = child->Get_Exit_Val ();
    ok = machine->WriteMem ((long) exitvalue, 4, (long) childexitvalue);
    // If the write failed (probably due to a page fault), try again.
    // If it still fails, abort the write
    if (!ok) {
      ok = machine->WriteMem ((long) exitvalue, 4, (long) childexitvalue);
    }
  }

  currentThread->Remove_Child ();
  pid = child->Get_Id();

  if (child->getStatus () == ZOMBIE) {
    delete child;
  } else {
        child->thread_exit_status = true;
	child->Set_Exit_Val (-childexitvalue); //FIXME : Since the exitvalue for child is written above into memory
					       // this Set_Exit_Val is solely for the purpose of signalling child's
					       // Exit status, to the parent, which thread_exit_status does.
					       //  So do we need this line ?? 
  }

  return (pid);
}


// ================================================================
// System_Exec:
// Parameters: register 4 contains a pointer to the filename of
//             the executable image.  
// Note: The executable image can take NO arguments.  
// Entry Point: Called from do_system_call
// ================================================================
int System_Exec (char * user_space_filename) {
  int ret;
  char * filename = new char[MAXFILENAMELENGTH];
 
  if (!filename) { 
    return -ENOMEM;
  }
  copy_from_user( user_space_filename, filename);
  currentThread->SetName(filename);
  
  OpenFile *executable = fileSystem->Open(filename);
  if ( executable == NULL ) {
    printf("Unable to open file %s\n", filename);
    delete [] filename;
    return -ENOENT;
  }

  ret = currentThread->space->ModifySpace(executable);
  //  delete executable;                  // close file
  delete [] filename;

  if (ret < 0) {
    return ret;
  }
  
  currentThread->space->InitRegisters();
  currentThread->space->RestoreState();  
  
  machine->Run();                     // jump to the user progam
  ASSERT(false);                      // machine->Run never returns;
  return 0;                           // make gcc happy
}


// ==========================================================================
// ==========================================================================
int System_Fork( ) {
  int pid;

  DSTRM_EVENT(SYSCALL, FORK, currentThread->Get_Id());

  // Create a new Thread object
  Thread *t = new Thread();
  if (!t) {
    return -EAGAIN;
  }
  // Creat an AddrSpace object for the thread
  AddrSpace *space = new AddrSpace (t);
  
  if (!space) {
    delete t;
    return -EAGAIN;
  }
  //Assign the address space to the thread
  t->space = space;
  if ( space->CopyFrom(t) < 0 ) {
    delete t;
    delete space;
    return -EAGAIN;
  }
  //Set the parent pointer for the new thread created as the current thread
  //as the current thread has forked a new child thread
  t->Set_Parent_Ptr (currentThread);
  for ( int i = 0; i < NumTotalRegs; i++ ) {
    t->Write_Reg( i, machine->ReadRegister(i) );
  }
  //calling the Thread::Fork function with Do_Fork as an argument
  pid = t->Fork((void(*)(size_t))Do_Fork, (size_t)0);
  
  //Add the number of children to the currentThread by incrementing the 
  //member variable Children of the Thread class
  currentThread->Add_Child ();

  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  // FIXME: Should this be here or in Thread::Fork()?
  // THIS should be here because the status of thread "t" can be set to 
  // "READY" only after all operations on it have been completed and it is
  // considered to be done only after the previous instruction finishes
  // adding the Children variable after returning from the Thread::Fork()
  // function allocating the stack and returning to this function.(System_Fork) 
  scheduler->ReadyToRun(t);              // ReadyToRun assumes that interrupts 
                                         // are disabled!
  (void) interrupt->SetLevel(oldLevel);

  return pid;
}


// ================================================================
// ================================================================
void System_Yield() {
  currentThread->Yield();
}


// ================================================================
// ================================================================
int System_GetPID (void) {
  return (currentThread->Get_Id ());
}


// ================================================================
// ================================================================
int System_GetPPID (void) {
  Thread *parent;

  parent = currentThread->Get_Parent_Ptr ();
  if (!parent) {
    return 0;
  }
  return (parent->Get_Id ());
}


// ================================================================
// This routine knows how to copy from the user space of the current thread
// to the designated place in kernel space
// ================================================================
int copy_from_user( char * from_user_space, char * to_k_space ) {
  int i = 0;

  do {
    bool ok;
    ok = machine->ReadMem ((long)(from_user_space++), 1, (long *)to_k_space);
    // If the read failed (probably due to a page fault), try again.
    // If it still fails, abort the read and return what was available
    if (!ok) {
      from_user_space--;
      ok = machine->ReadMem((long)(from_user_space++), 1, (long *)to_k_space);
      if (!ok) {
	*to_k_space = '\0';
	break;
      }
    }
    i++;
  }
  while (*(to_k_space++) && (i < MAXFILENAMELENGTH));
  return i;
}


// ==========================================================================
// DO FORK IS RIGHT HERE:
// ==========================================================================
/* This function is passed as an argument to Thread::Fork() from System_Fork()
   above */
void Do_Fork(size_t dummy) {
  dummy = 0;  // Keep gcc happy; the mechanism we use to call this only works
	      // with functions that take 1 size_t argument
  
  currentThread->RestoreUserState();
  currentThread->space->RestoreState();

  /* Executed by the child process.  Sets the registers to the state they
     would have if the operating system was executing on the real hardware,
     which it isn't. */
  machine->Fix_Fork_Registers();

  machine->Run();
}


// ===============================================================
// System_Nice:
// Parameters: register 4 contains an int containing the
// increment value for the priority
// Entry Point: Called from do_system_call
// ===============================================================
void System_Nice (int inc) {
    DSTRM_EVENT(SYSCALL, NICE, currentThread->Get_Id());
	DEBUG((char *) DB_NICE, (char *) "\nNice(%s, %d)\n", 
		  currentThread->GetName(), inc);
    currentThread->Prioritize(inc);
}



int System_NameThread(char *user_space_thread_name)
{
  char *name = new char[MAXFILENAMELENGTH];
  
  if (!name) { 
    return -ENOMEM;
  }

  copy_from_user(user_space_thread_name, name);
  DEBUG((char *) DB_NICE, (char *) "\nNameThread(\"%s\" -> \"%s\")\n", 
		currentThread->GetName(), name);
  
  DSTRM_EVENT_DATA(THREAD, NAME_THREAD, currentThread->Get_Id(), 
		   strlen(name), (void *)name, "print_string");


  currentThread->SetName(name);
  return 0;
}


/* -------------------------------------------------------------- */
void System_NachosUserEvent(char *u_family, char *u_name, int tag)
{
  char *family = new char[48];
  char *name = new char[48];
  struct datastream_ip *ip;

  copy_from_user(u_name, name);
  copy_from_user(u_family, family);

  ip = dsui_get_ip_byname(family, name);

  if (!ip) {
	ip = dsui_create_ip(family, name,
						DS_EVENT_TYPE, (char *)"print_int");
	dsui_enable_ip(0, ip, NULL);	
  }

  if (*ip->next) {
	int pid = currentThread->Get_Id();
	dsui_event_log(ip, tag, sizeof(pid), &pid);
  }
}
/* -------------------------------------------------------------- */




// ================================================================
// System_Echo:
// Parameters: register 6 contains the # of bytes to write.  Register
//             5 contains a pointer to the string to write.
// Returns: # of bytes written in register 2.
// This is a testing variant of the the write system call, it
// circumvents the NachOS console device.  It is called from
// do_system_call.
// ================================================================
int System_Echo (char * from_user_space, int num_to_write) {
  int byteswritten;
  char* buffer;
  
  buffer = new char[num_to_write + 1];
  
  if (buffer == NULL) {
    return -ENOMEM;
  }

  for (int i = 0; i < num_to_write; i++) {
    bool ok;
    ok = machine->ReadMem ((long)(from_user_space + i), 1, (long *)&buffer[i]);
    // If the read failed (probably due to a page fault), try again.
    // If it still fails, abort the read and write what was available
    if (!ok) {
      ok = machine->ReadMem((long)(from_user_space + i), 1, (long *)&buffer[i]);
      if (!ok) {
	num_to_write = i;
	break;
      }
    }
  }
  buffer [num_to_write] = '\0';
  
  byteswritten = write (1, buffer, num_to_write); // write to stdout (i.e. the NachOS simulator's stdout)
  
  delete [] buffer;
  return byteswritten;
}

#endif /* USER_PROGRAM */

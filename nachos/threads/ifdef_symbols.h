
//Include the REMOTE_USER_PROGRAM_DEBUGGING flag
//which is used to implement the nachos remote debugging feature
// was defined in the makefile in userprog in the DEFINES variable
//Now we include it here to decide whether to incroporate it as a 
//whole within the nachos code or just leave the changes within the flag

#ifndef REMOTE_USER_PROGRAM_DEBUGGING
#define REMOTE_USER_PROGRAM_DEBUGGING
#endif


//flag to include some error message in 
//functions Machine::ReadMem and WriteMem
// using DEBUG routine. in translate.cc

#ifndef F_ERRORMSG
#define F_ERRORMSG
#endif

//flag to change Sleep() to Sleep(BLOCKED)
//and Sleep(ThreadStatus newstatus) --> killThread(ThreadStatus threadstate)
//in threads/synch.cc , thread.cc and thread.h and userprog/systemcall.cc

#ifndef F_SLEEP
#define F_SLEEP
#endif



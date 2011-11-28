// systemcall.h:
// header file for all the system functions.
//


#ifndef __SYSTEM_CALL
#define __SYSTEM_CALL



#ifdef USER_PROGRAM

#ifndef SYSCALL_CHANGE
#include "syscallnumbers.h"

extern void do_system_call (int syscall_num);
extern int System_Read (int from_fd, char * to_user_space, int num_to_read);
extern int System_Write (int to_fd, char * from_user_space, int num_to_write);
extern int System_Fork (void);
extern int System_Exec (char * user_space_filename);
extern int System_Wait (int *exitvalue);   //not done
extern int System_Create (char *user_space_filename);
extern int System_Open (char *user_space_filename);
extern int System_Close (int fd);
extern int System_Unlink (char *user_space_filename);
extern int System_GetPID (void);
extern int System_GetPPID (void);
extern void System_Yield (void);
extern void System_Halt (void);
extern void System_Exit (int exitvalue);

/* ----------------------------------------------- */
extern void System_NachosUserEvent(char *family, char *name, int pid);
/* ----------------------------------------------- */

extern int System_Echo (char * from_user_space, int num_to_write);
extern void System_Nice (int inc);

extern int System_NameThread (char *name);

extern int copy_from_user (char * from_user_space, char * to_k_space);
extern void Do_Fork (size_t dummy);

#endif /* SYSCALL_CHANGE */

#endif /* USER_PROGRAM */

#endif /* SYSTEM_CALL */

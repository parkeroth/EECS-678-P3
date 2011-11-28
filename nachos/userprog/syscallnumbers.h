
//SYSCALL_CHANGE
/* Includes the  #define constants which were
 * in syscall.h and systemcall.h
 */
#ifndef SYSCALL_CHANGE

#define SC_Halt         0
#define SC_Exit         1
#define SC_Exec         2
#define SC_Wait         3
#define SC_Create       4
#define SC_Open         5
#define SC_Read         6
#define SC_Write        7
#define SC_Close        8
#define SC_Fork         9
#define SC_Yield        10
#define SC_Unlink       11
#define SC_GetPID       12
#define SC_GetPPID      13
#define SC_Nice         14

#define SC_Echo         15

#define SC_NachosUserEvent    16

#define SC_NameThread       25

#endif

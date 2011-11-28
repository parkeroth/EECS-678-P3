#ifndef DEFS_H
#define DEFS_H

//defines the Maximum File Name length
#define MAXFILENAMELENGTH 80 

//defines the maximum File Descriptor number that can be used
#define MAX_FD            16

// defines the states of a thread
enum ThreadStatus { JUST_CREATED, RUNNING, READY, BLOCKED, ZOMBIE, NUM_THREAD_STATES };

#endif

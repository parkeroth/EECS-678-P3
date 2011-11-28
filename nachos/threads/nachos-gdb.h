#ifndef __NACHOS_GDB__
#define __NACHOS_GDB__

struct nachos_thread {
  void *thread;
  int ID;
  struct nachos_thread *next;
};

struct nachos_thread_list_struct {
  struct nachos_thread *head;
  struct nachos_thread *tail;
};

typedef struct nachos_thread_list_struct nachos_thread_list;

extern nachos_thread_list allThreads;

#endif

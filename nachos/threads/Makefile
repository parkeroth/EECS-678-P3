# NOTE: this is a GNU Makefile.  You must use "gmake" rather than "make".
#
# Makefile for the threads assignment.  The threads assignment must
#  be done first!
#
# Copyright (c) 1992 The Regents of the University of California.
# All rights reserved.  See copyright.h for copyright notice and limitation 
# of liability and disclaimer of warranty provisions.

DEFINES = -DTHREADS -DSMARTGDB
INCPATH = -I../threads -I../machine -I../userprog -I../filesys
HFILES = $(THREAD_H)
CFILES = $(THREAD_C)
C_OFILES = $(THREAD_O)

Syscalls.s: ../userprog/Systemcalls.s
	$(CPP) $(CFLAGS) ../userprog/Systemcalls.s > Syscalls.s

strt.s: ../userprog/start.s
	$(CPP) $(CFLAGS) ../userprog/start.s > strt.s

include ../Makefile.dep
include ../Makefile.common
-include .depend

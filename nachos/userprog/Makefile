# NOTE: this is a GNU Makefile.  You must use "gmake" rather than "make".
#
# Makefile for the multiprogramming assignment
#    Defines set up assuming multiprogramming is done before the file system.
#    If not, use the "filesys first"  defines below. 
#
#
# Copyright (c) 1992 The Regents of the University of California.
# All rights reserved.  See copyright.h for copyright notice and limitation 
# of liability and disclaimer of warranty provisions.

IS_LOCAL_INSTALL = no

KUSP_INCLUDE_FLAGS = 

ifeq ($(IS_LOCAL_INSTALL), yes)
	KUSP_INCLUDE_FLAGS = -I/opt/kusp/include
	KUSP_LIBRARY_DIR = -L/opt/kusp/lib
endif

DSUI_INCLUDE_ARG = -I../dsui_tmp -I../dsui-sched -I../dsui-vm
DSUI_DEFINE      = -DCONFIG_DSUI

DEFINES = -DUSER_PROGRAM -DFILESYS_NEEDED -DFILESYS_STUB -DSMARTGDB $(DSUI_INCLUDE_ARG)
#FIXME - KUSP_INC = -I$(KUSP_INCLUDE)
INCPATH = -I../bin -I../filesys -I../userprog -I../threads -I../machine -I../debug -I../network $(DSUI_DEFINE) $(KUSP_INCLUDE_FLAGS)
#FIXME - INCPATH = -I../bin -I../filesys -I../userprog -I../threads -I../machine -I../debug -I../network $(DSUI_DEFINE) $(KUSP_INC)
HFILES = $(THREAD_H) $(USERPROG_H) $(DEBUG_H)
CFILES = $(THREAD_C) $(USERPROG_C) $(DEBUG_C)
C_OFILES = $(THREAD_O) $(USERPROG_O) $(DEBUG_O)

#SYSCALL_CHANGE
#Use it if we need start.o obj file in userprog itself
#GCCDIR = /usr/local/nachos/bin/
#CC = $(GCCDIR)mips-dec-ultrix4.2-gcc
#ASM = $(GCCDIR)mips-dec-ultrix4.2-as
#LD = $(GCCDIR)mips-dec-ultrix4.2-ld

#CPP = /lib/cpp
#INCDIR =-I../userprog -I../threads -I../lib
#CPPFLAGS = $(INCDIR)
#ASMFLAGS =

#Systemcalls.o: Systemcalls.s
#       $(CPP) $(CPPFLAGS)  Systemcalls.s > Syscalls.s
#       $(ASM) $(ASMFLAGS) -o Systemcalls.o Syscalls.s
#       rm Syscalls.s

#start.o: start.s
#       $(CPP) $(CPPFLAGS) start.s > strt.s
#       $(ASM) $(ASMFLAGS) -o start.o strt.s
#       rm strt.s

#SYSCALL_CHANGE OVER

# if file sys done first!
# DEFINES = -DUSER_PROGRAM -DFILESYS_NEEDED -DFILESYS
# INCPATH = -I../bin -I../filesys -I../userprog -I../threads -I../machine
# HFILES = $(THREAD_H) $(USERPROG_H) $(FILESYS_H)
# CFILES = $(THREAD_C) $(USERPROG_C) $(FILESYS_C)
# C_OFILES = $(THREAD_O) $(USERPROG_O) $(FILESYS_O)

include ../Makefile.dep
include ../Makefile.common
-include .depend



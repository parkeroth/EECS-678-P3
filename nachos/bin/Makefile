# Use normal make for this Makefile
#
# Makefile for:
#	disassemble -- disassembles a normal MIPS executable 
#
# Copyright (c) 1992 The Regents of the University of California.
# All rights reserved.  See copyright.h for copyright notice and limitation 
# of liability and disclaimer of warranty provisions.

# If the host is big endian (SPARC, SNAKE, etc):
# change to (disassemble and coff2flat don't support big endian yet):
# CFLAGS= -I./ -I../threads -DHOST_IS_BIG_ENDIAN
# all: coff2noff

CC=gcc
CFLAGS=-I./ -I../threads
LD=gcc

all: coff2noff

# converts a COFF file to a flat address space (for Nachos version 2)
coff2flat: coff2flat.o
	$(LD) coff2flat.o -o coff2flat

# disassembles a COFF file
disassemble: out.o opstrings.o
	$(LD) out.o opstrings.o -o disassemble

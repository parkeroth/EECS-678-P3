# use normal make for this Makefile
#
# Makefile for building library routines
#

# if you are cross-compiling, you need to point to the right executables
# and change the flags to ld and the build procedure for as
GCCDIR = /usr/local/nachos/bin/
LDFLAGS = # -T script -N
ASFLAGS = # -mips
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)

CC = $(GCCDIR)mipsel-elf-gcc
AS = $(GCCDIR)mipsel-elf-as
LD = $(GCCDIR)mipsel-elf-ld
RANLIB = $(GCCDIR)mipsel-elf-ranlib

INCDIR =-I../userprog -I../threads
CFLAGS = -fno-builtin -nostdinc $(INCDIR)

OBJ = strlen.o itoa.o strcat.o strcpy.o strcmp.o strncmp.o strcat.o strncmp.o\
	strchr.o strrchr.o strstr.o atoi.o gets.o memcpy.o memset.o memchr.o

all : lib.a

%.o: %.c
	$(CC) $(CFLAGS) -c $<

lib.a : $(OBJ)
	$(AR) rc lib.a $(OBJ)
	$(RANLIB) lib.a

clean :
	rm -f *.o lib.a

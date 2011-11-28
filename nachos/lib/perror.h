#include "syscall.h"

/*
 * The perror routine below shows some of the current limitations with nachos.
 * Because of the references to other routines in the library (strcpy, etc),
 * in combination with the static strings, gcc is unable to compile this if
 * it is created as perror.c and linked into lib.a.  Therefore, the only way
 * to use this routine for now is to #include it in your main source file.
 */

char perror_buffer[80];
extern int errno;

void perror (char *s) {
  char *errmsg;

  strcpy (perror_buffer, s);
  strcat (perror_buffer, ": ");

  switch (errno) {
    case 2:  
	errmsg = "No such file or directory";
	break;
    case 9:  
	errmsg = "Bad file number";
	break;
    case 10:  
	errmsg = "No child processes";
	break;
    case 11:
	errmsg = "Try again";
	break;
    case 12:
	errmsg = "Out of memory";
	break;
    case 24:
	errmsg = "Too many open files";
	break;
    default:
	errmsg = "";
	break;
  }

  strcat (perror_buffer, errmsg);
  strcat (perror_buffer, "\n");
  Write (ConsoleOutput, perror_buffer, strlen (perror_buffer));
}


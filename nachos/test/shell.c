#include "syscall.h"
#include "stdlib.h"
#include "perror.h"

extern int errno;

int main (void) {
  int newProc;
  OpenFileId output = ConsoleOutput;
  const char *prompt = "nachos> ";
  char inbuf[60], outbuf[60];
  int i, retval, background = 0, exitval;
  
  while (1) {
    retval = Write (output, prompt, strlen (prompt));
    
    i = 0;
    
    gets (inbuf);

    if (strlen (inbuf) < 1) {
      continue;
    }

    if (inbuf[strlen(inbuf) - 1] == '&') {
      background = 1;
      inbuf[strlen(inbuf) - 1] = '\0';
    } else {
      background = 0;
    }

    if (!strcmp (inbuf, "exit")) {
      Exit (0);
    } else if (!strcmp (inbuf, "wait")) {
      retval = Wait (&exitval);
      if (retval >= 0) {
        strcpy (outbuf, "Wait returned ");
        itoa (retval, outbuf + strlen (outbuf));
	strcat (outbuf, ".  Exit value = ");
	itoa (exitval, outbuf + strlen (outbuf));
        strcat (outbuf, "\n");
        Write (output, outbuf, strlen (outbuf));
      } else {
        perror ("Wait");
      }
    } else if (strcmp (inbuf, "")) {
      if ((retval = Fork ()) == 0) {
	newProc = Exec (inbuf);
	
	if (newProc < 0) {
          perror ("Exec");
	  Exit (-1);
	}
      }
      if (retval < 0) {
	perror ("Fork");
      }
      
      if (!background) {
	retval = Wait (&exitval);
        if (retval >= 0) {
          strcpy (outbuf, "Wait returned ");
          itoa (retval, outbuf + strlen (outbuf));
          strcat (outbuf, ".  Exit value = ");
          itoa (exitval, outbuf + strlen (outbuf));
          strcat (outbuf, "\n");
          Write (output, outbuf, strlen (outbuf));
        } else {
          perror ("Wait");
        }
      }
    }
  }
}

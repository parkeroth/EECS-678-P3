// utility.cc 
//	Debugging routines.  Allows users to control whether to 
//	print DEBUG statements, based on a command line argument.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#pragma implementation "threads/utility.h"

#include "copyright.h"
#include "utility.h"
#include <stdarg.h>

static char *enableTags = NULL; // controls which DEBUG messages are printed 

//----------------------------------------------------------------------
// DebugInit
//      Initialize so that only DEBUG messages with a tag in tagList 
//	will be printed.
//
//	If the tag includes "all", all DEBUG messages are enabled.
//
// 	"tagList" is a colon-delimited list of flags whose DEBUG
// 	messages are to be enabled.
//
//      "enableTags" is a static-scope (this file only) variable used
//      to reference dynamic memory. It contains a slightly modified
//      version of "tagList" which is easier to query".
//----------------------------------------------------------------------

void
DebugInit(char *tagList)
{
  enableTags = new char[ strlen(tagList) + 3 ];
  enableTags[0] = '\0';

  // add a colon on each side of the tagList, this simplifies the
  // lookup used to determine when a flag is enabled
  strcat( enableTags , ":" );
  strcat( enableTags , tagList );
  strcat( enableTags , ":" );
}

//----------------------------------------------------------------------
// DebugIsEnabled
//      Return true if DEBUG messages with "tag" are to be printed.
//----------------------------------------------------------------------

bool
DebugIsEnabled(char* tag)
{
  const size_t BUFLEN = 258;
  char buffer[BUFLEN] = ":";

  if ( NULL == enableTags )
    return false;

  if ( NULL != strstr( enableTags , ":all:" ) )
    return true;

  ASSERT( strlen( tag ) <= (BUFLEN-2) ) ;

  strcat( buffer , tag );
  strcat( buffer , ":" );

  return ( NULL != strstr( enableTags , buffer ) );
}

//----------------------------------------------------------------------
// DEBUG
//      Print a debug message, if tag is enabled.  Like printf,
//	only with an extra argument on the front.
//----------------------------------------------------------------------

void 
DEBUG(char* tag, char *format, ...)
{
  if (DebugIsEnabled(tag))
    {
      va_list ap;

      va_start(ap, format);
      vfprintf(stdout, format, ap);
      va_end(ap);
      fflush(stdout);
    }
}

//----------------------------------------------------------------------
// DebugCleanUp
//      The list of enabled flags is managed in dynamic memory, we
//      must release it.
//----------------------------------------------------------------------

void
DebugCleanUp()
{
  delete [] enableTags;
  enableTags = NULL;
}

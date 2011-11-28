// console.cc 
//	Routines to simulate a serial port to a console device.
//	A console has input (a keyboard) and output (a display).
//	These are each simulated by operations on UNIX files.
//	The simulated device is asynchronous,
//	so we have to invoke the interrupt handler (after a simulated
//	delay), to signal that a byte has arrived and/or that a written
//	byte has departed.
//
//  DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#pragma implementation "machine/console.h"

#include "copyright.h"
#include "console.h"
#include "system.h"

// Write arguments
struct console_write_request {
  Console *console;
  char character;
  int fd;
};

// Dummy functions because C++ is weird about pointers to member functions
static void ConsoleReadPoll(size_t c) 
{ Console *locConsole = (Console *)c; locConsole->CheckCharAvail(); }
static void ConsoleFinishWrite(size_t c)
{
  console_write_request *req = reinterpret_cast<console_write_request *>( c );
  WriteFile( req->fd , &( req->character ) , sizeof( req->character ) ); // Print the character.
  req->console->WriteDone();

  // done with request
  delete req;
}


// ConsoleRead()
//
// Read a specified number of bytes from the console.
//
// Arguments:
// buf      : Buffer in which to place the bytes read (must be big enough
//            to hold the requested number of bytes).
// nbytes   : The number of bytes to read
//
// Return value:
// The number of bytes read, or a negative number in case of an error.

int ConsoleRead(char *buf, int nbytes) {
  int bytesread = 0;

  consoleRead->P();

  while (bytesread < nbytes)
    {
      consoleReadAvail->P();  // Put on the semaphore queue for console read.
      buf[bytesread] = console->GetChar(); // Read a character.
      if (buf[bytesread] == EOF)
	{
	  break;
	}
      bytesread++; // Count the byte I just read.
    }

  consoleRead->V();

  return bytesread;
}


// ConsoleWrite() 
//
// Write a specified number of bytes to the console.
//
// Arguments:
// buf      : The buffer to read from
// nbytes   : The number of bytes to write
//
// Return value:
// The number of bytes written, or a negative number in case of an error.

int ConsoleWrite(char *buf, int nbytes) {
  int byteswritten = 0;

  consoleWrite->P();

  while (byteswritten < nbytes)
    {
      console->PutChar(buf[byteswritten]); // Interact with the console.
      consoleWriteDone->P(); // Wait until it has been written.
      byteswritten++; // Count the byte just written.
    }

  consoleWrite->V();

  return byteswritten;
}


//----------------------------------------------------------------------
// Console::Console
// 	Initialize the simulation of a hardware console device.
//
//	"readFile" -- UNIX file simulating the keyboard (NULL -> use stdin)
//	"writeFile" -- UNIX file simulating the display (NULL -> use stdout)
// 	"readAvail" is the interrupt handler called when a character arrives
//		from the keyboard
// 	"writeDone" is the interrupt handler called when a character has
//		been output, so that it is ok to request the next char be
//		output
//----------------------------------------------------------------------

Console::Console(char *readFile, char *writeFile, VoidFunctionPtr readAvail, 
		 VoidFunctionPtr writeDone, size_t callArg)
{
  if (readFile == NULL)
    readFileNo = 0;					// keyboard = stdin
  else
    readFileNo = OpenForReadWrite(readFile, true);	// should be read-only
  if (writeFile == NULL)
    writeFileNo = 1;					// display = stdout
  else
    writeFileNo = OpenForWrite(writeFile);

  // set up the stuff to emulate asynchronous interrupts
  writeHandler = writeDone;
  readHandler = readAvail;
  handlerArg = callArg;
  putBusy = false;
  incoming = FLAG_EOF;

  // start polling for incoming packets
  interrupt->Schedule(ConsoleReadPoll, (size_t)this, ConsoleTime,
		      ConsoleReadInt);
}

//----------------------------------------------------------------------
// Console::~Console
// 	Clean up console emulation
//----------------------------------------------------------------------

Console::~Console()
{
  if (readFileNo != 0)
    Close(readFileNo);
  if (writeFileNo != 1)
    Close(writeFileNo);
}

//----------------------------------------------------------------------
// Console::CheckCharAvail()
// 	Periodically called to check if a character is available for
//	input from the simulated keyboard (eg, has it been typed?).
//
//	Only read it in if there is buffer space for it (if the previous
//	character has been grabbed out of the buffer by the Nachos kernel).
//	Invoke the "read" interrupt handler, once the character has been 
//	put into the buffer. 
//----------------------------------------------------------------------

void
Console::CheckCharAvail()
{
  char c;

  // schedule the next time to poll for a packet
  interrupt->Schedule(ConsoleReadPoll, (size_t)this, ConsoleTime, 
		      ConsoleReadInt);

  // do nothing if character is already buffered, or none to be read
  if (incoming != FLAG_EOF) 
    return;
  if (!PollFile(readFileNo))
    return;
  // otherwise, read character and tell user about it
  incoming = (ReadPartial(readFileNo, &c, sizeof(c)) == sizeof(c)) ? c : EOF;
  stats->numConsoleCharsRead++;
  if (currentThread) {
    currentThread->procStats->numConsoleCharsRead++;
  }
  (*readHandler)(handlerArg);	
}

//----------------------------------------------------------------------
// Console::WriteDone()
// 	Internal routine called when it is time to invoke the interrupt
//	handler to tell the Nachos kernel that the output character has
//	completed.
//----------------------------------------------------------------------

void
Console::WriteDone()
{
  putBusy = false;
  stats->numConsoleCharsWritten++;
  if (currentThread)
    {
      currentThread->procStats->numConsoleCharsWritten++;
    }
  (*writeHandler)(handlerArg);
}

//----------------------------------------------------------------------
// Console::GetChar()
// 	Read a character from the input buffer, if there is any there.
//	Either return the character, or EOF if none buffered.
//----------------------------------------------------------------------

char
Console::GetChar()
{
  char ch = incoming;

  incoming = FLAG_EOF;
  return ch;
}

//----------------------------------------------------------------------
// Console::PutChar()
// 	Schedule an interrupt to occur (which will write the character
// AND release memory) in the future, and return.
//----------------------------------------------------------------------

void
Console::PutChar(char ch)
{
  ASSERT(!putBusy);

  // the console is busy until its done with this request
  putBusy = true;

  // prepare request
  // (interrupt handler will release the memory)
  console_write_request *req = new console_write_request;
  req->console = this;
  req->character = ch;
  req->fd = writeFileNo;

  // schedule the request to finish in ConsoleTime ticks
  interrupt->Schedule(ConsoleFinishWrite, (size_t)req, ConsoleTime,
		      ConsoleWriteInt);
}

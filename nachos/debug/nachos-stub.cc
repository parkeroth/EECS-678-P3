/*
 * File: nachos-stub.cc
 * Author: Sean B House, University of Kansas
 *         Heavily hacked, modified, updated, and enhanced by Jerry James
 * Description: This is the Nachos (MIPS) GDB stub that facilitates
 *              debugging of Nachos user-level programs through GDB
 *              remote debugging.
 */

#pragma implementation "debug/nachos-stub.h"

#include <errno.h>
#include <signal.h>
#include "system.h"
#include "utility.h"

#ifdef USER_PROGRAM

static const char hexchars[] = "0123456789abcdef";

/* Table used by the crc32 function to calcuate the checksum. */
static unsigned long crc32_table[256] =
{0, 0};


/*
 * Method: GDBRemoteDebugger
 * Description: This is the constructor for the GDB remote debugger.
 */
GDBRemoteDebugger::GDBRemoteDebugger (int port) {
  unsigned int cliAddrLen;

  if ((sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      DEBUG( (char *)DB_RGDB , (char *)"socket() failed");
      currentThread->Finish();
      ASSERT(false);
    }

  memset(&serverAddr, '\0', sizeof(serverAddr));

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddr.sin_port = htons(port);

  if (bind(sockFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
      DEBUG( (char *)DB_RGDB , (char *)"bind() failed");
      currentThread->Finish();
      ASSERT(false);
    }

  if (listen(sockFD, 1) < 0)
    {
      DEBUG( (char *)DB_RGDB , (char *)"listen() failed"); 
      currentThread->Finish();
      ASSERT(false);
    }

  cliAddrLen = sizeof(clientAddr);

  printf("Waiting for a connection from GDB...\n");    

  do
    {
      newSockFD = accept(sockFD, (struct sockaddr *)&clientAddr, &cliAddrLen);
    }
  while (newSockFD < 0 && errno == EINTR);

  if (newSockFD < 0)
    {
      DEBUG( (char *)DB_RGDB , (char *)"accept() failed");
      currentThread->Finish();
      ASSERT(false);
    }

  printf("Connection established...\n");    
  Initialize();
}

/*
 * Method: Exit
 * Description: This function informs GDB that the program has exited
 *              successfully.
 */
void
GDBRemoteDebugger::Exit () {
  /* Tell GDB that the process exited normally. */
  remcomOutBuffer[0] = 'W';
  remcomOutBuffer[1] = '0';
  remcomOutBuffer[2] = '0';
  remcomOutBuffer[3] = '\0';
  PutPacket();
}

/*
 * Method: Initialize
 * Description: This function initializes the stub and puts the MIPS simulated
 *              machine into single step mode.  This is how we initially give
 *              GDB control.
 */
void
GDBRemoteDebugger::Initialize () {
  DEBUG( (char *)DB_RGDB , (char *)"Initializing GDBRemoteDebugger...\n");
  machine->singleStep = true;
}

/*
 * Method: PutDebugChar
 * Description: This function writes a single character to the socket.
 */
int
GDBRemoteDebugger::PutDebugChar (char ch) {
  return write(newSockFD, &ch, 1);
}

/*
 * Method: GetDebugChar
 * Description: This function reads a single character from the socket.
 */
char
GDBRemoteDebugger::GetDebugChar () {
  char ch;

  read(newSockFD, &ch, 1);
  return ch;
}

/*
 * Method: GDBCatchException
 * Description: This function is the hook into the stub from the mips machine.
 */
void
GDBRemoteDebugger::GDBCatchException (int number) {
  DEBUG( (char *)DB_RGDB , (char *)"Caught exception %d\n", number);

  SaveRegisters();
  HandleException(number);
}

/*
 * Method: SaveRegisters
 * Description: This function saves the register state.
 */
void
GDBRemoteDebugger::SaveRegisters () {
  register int i;

  for (i = 0; i < NumGPRegs; i++)
    {
      registers[i] = machine->ReadRegister(i);
    }

  registers[LoReg] = machine->ReadRegister(LoReg);
  registers[HiReg] = machine->ReadRegister(HiReg);
  registers[GDBBadVAddrReg] = machine->ReadRegister(BadVAddrReg);
  registers[GDBPCReg] = machine->ReadRegister(PCReg);
}

/*
 * Method: RestoreRegisters
 * Description: This function restores the register state.
 */
void
GDBRemoteDebugger::RestoreRegisters () {
  register int i;

  for (i = 0; i < NumGPRegs; i++)
    {
      machine->WriteRegister(i, registers[i]);
    }

  machine->WriteRegister(LoReg, registers[LoReg]);
  machine->WriteRegister(HiReg, registers[HiReg]);
  machine->WriteRegister(BadVAddrReg, registers[GDBBadVAddrReg]);
  machine->WriteRegister(PCReg, registers[GDBPCReg]);
}

/*
 * Method: Hex
 * Description: This function converts a hex digit into an integer.
 */
int
GDBRemoteDebugger::Hex (char ch) {
  if (ch >= 'a' && ch <= 'f') return (ch - 'a' + 10);
  if (ch >= '0' && ch <= '9') return (ch - '0');
  if (ch >= 'A' && ch <= 'F') return (ch - 'A' + 10);

  return -1;
}

unsigned long
GDBRemoteDebugger::CRC32 (unsigned char *buf, int len, unsigned int crc) {
  if (!crc32_table[1])
    {
      /* Initialize the CRC table and the decoding table. */
      int i, j;
      unsigned int c;

      for (i = 0; i < 256; i++)
	{
	  for (c = i << 24, j = 8; j > 0; --j)
	    c = c & 0x80000000 ? (c << 1) ^ 0x04c11db7 : (c << 1);
	  crc32_table[i] = c;
	}
    }

  while (len--)
    {
      crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *buf) & 255];
      buf++;
    }
  return crc;
}

/*
 * Method: ReadMemory
 * Description: This function reads memory from the simulated machine.
 */
bool
GDBRemoteDebugger::ReadMemory (int addr, int length, int *value) {
  const int num = length / 4;
  const int remainder = length & 0x3;
  register bool success = false;
  register int i;

  for (i = 0; i < num; i++)
    {
      if (!(success = machine->ReadMem(addr, 4, &value[i])))
	if (!(success = machine->ReadMem(addr, 4, &value[i])))
	  {
	    DEBUG( (char *)DB_RGDB , (char *)"Read from memory failed twice...\n");
	    return success;
	  }

      if (success)
	{
	  DEBUG( (char *)DB_RGDB , (char *)"Read 4 bytes at %d: %u\n", addr,
		(unsigned int)(value[i]));
	  addr += 4;
	}
    }

  if (remainder)
    {
      if (!(success = machine->ReadMem(addr, remainder, &value[i])))
	if (!(success = machine->ReadMem(addr, remainder, &value[i])))
	  {
	    DEBUG( (char *)DB_RGDB , (char *)"Read from memory failed twice...\n");
	    return success;
	  }

      if (success)
	DEBUG( (char *)DB_RGDB , (char *)"Read %d bytes at %d: %u\n", remainder, addr,
	       (unsigned int)(value[i]));
    }

  return success;
}

/*
 * Method: WriteMemory
 * Description: This function reads the memory of the simulated machine.
 */
bool
GDBRemoteDebugger::WriteMemory (int addr, int length, int * value) {
  const int num = length / 4;
  const int remainder = length & 0x3;
  register bool success = false;
  register int i;

  for (i = 0; i < num; i++)
    {
      if (!(success = machine->WriteMem(addr, 4, value[i])))
	if (!(success = machine->WriteMem(addr, 4, value[i])))
	  {
	    DEBUG( (char *)DB_RGDB , (char *)"Write to memory failed twice...\n");
	    return success;
	  }

      if (success)
	{
	  DEBUG( (char *)DB_RGDB , (char *)"Writing 4 bytes at %d: %u\n", addr,
		 (unsigned int)(value[i]));
	  addr += 4;
	}
    }

  if (remainder)
    {
      if (!(success = machine->WriteMem(addr, remainder, value[i])))
	if (!(success = machine->WriteMem(addr, remainder, value[i])))
	  {
	    DEBUG( (char *)DB_RGDB , (char *)"Write to memory failed twice...\n");
	    return success;
	  }

      if (success)
	DEBUG( (char *)DB_RGDB , (char *)"Writing %d bytes at %d: %u\n", remainder, addr,
	       (unsigned int)(value[i]));
    }

  return success;
}

/*
 * Method: GetPacket
 * Description: This function scans for the sequence $<data>#<checksum>
 */
void
GDBRemoteDebugger::GetPacket () {
  register unsigned char checksum, xmitcsum;
  register int count;
  register char ch;

  for (;;)
    {
      /* wait around for the start character, ignore all other characters */
      while ((ch = GetDebugChar() & 0x7f) != '$') ;

    retry:
      checksum = 0;
      xmitcsum = (unsigned char)-1;
      count = 0;

      /* now, read until a # or end of buffer is found */
      while (count < BUFMAX)
	{
	  ch = GetDebugChar() & 0x7f;
	  if (ch == '$')
	    goto retry;
	  if (ch == '#')
	    break;
	  checksum += ch;
	  remcomInBuffer[count] = ch;
	  count++;
	}
      remcomInBuffer[count] = '\0';

      if (ch == '#')
	{
	  xmitcsum = Hex(GetDebugChar() & 0x7f) << 4;
	  xmitcsum += Hex(GetDebugChar() & 0x7f);

	  if (checksum != xmitcsum)
	    {
	      DEBUG( (char *)DB_RGDB , (char *)"Bad checksum. My count = 0x%x, sent=0x%x, buf=%s\n",
		     checksum, xmitcsum, remcomInBuffer);
	      PutDebugChar('-');            /* failed checksum */
	    }
	  else
	    {
	      PutDebugChar('+');            /* successful transfer */
	      return;
	    }
	}
    }
}

/*
 * Method: PutPacket
 * Description: This function writes $<packet info>#<checksum>
 */
void
GDBRemoteDebugger::PutPacket () {
  register unsigned char checksum;
  register int count;
  register char ch;
    
  DEBUG( (char *)DB_RGDB , (char *)"Sending: %s\n", remcomOutBuffer);
    
  do
    {
      PutDebugChar('$');
      checksum = 0;
      count = 0;

      while ((ch = remcomOutBuffer[count]))
	{
	  if (!PutDebugChar(ch))
	    return;
	  checksum += ch;
	  count++;
	}

      PutDebugChar('#');
      PutDebugChar(hexchars[checksum >> 4]);
      PutDebugChar(hexchars[checksum & 0xf]);
    }
  while ((GetDebugChar() & 0x7f) != '+');   
}

/*
 * Method: MemToHex
 * Description: This function converts the contents of memory to hexadecimal.
 */
char *
GDBRemoteDebugger::MemToHex (unsigned char *mem, char *buf, int count) {
  register unsigned char ch;

  while (count-- > 0)
    {
      ch = *mem++;
      *buf++ = hexchars[ch >> 4];
      *buf++ = hexchars[ch & 0x0f];
    }

  *buf = '\0';
  return buf;
}

/*
 * Method: HexToMem
 * Description: This function converts hexadecimal to memory.
 */
char *
GDBRemoteDebugger::HexToMem (unsigned char *buf, char *mem, int count) {
  register unsigned char ch;
  register int i;

  for (i = 0; i < count; i++)
    {
      ch = Hex(*buf++) << 4;
      ch |= Hex(*buf++);
      *mem++ = ch;
    }

  return mem;
}

/*
 * Method: HexToInt
 * Description: This function converts hexadecimal to an integer.
 */
int
GDBRemoteDebugger::HexToInt (char **ptr, int *intValue) {
  register int numChars = 0;
  register int hexValue;

  *intValue = 0;

  while (**ptr)
    {
      hexValue = Hex(**ptr);
      if (hexValue < 0)
	break;

      *intValue = (*intValue << 4) | hexValue;
      numChars++;

      (*ptr)++;
    }

  return numChars;
}

/*
 * Method: ComputeSignal
 * Description: This function translates the exception into a 
 *              Unix compatible signal value.
 */
int
GDBRemoteDebugger::ComputeSignal (enum ExceptionType exception) {
  switch (exception)
    {
    case           NoException: return         0;
    case      SyscallException: return    SIGBUS;
    case    PageFaultException: return   SIGSEGV;
    case     ReadOnlyException: return    SIGBUS;
    case     BusErrorException: return    SIGBUS;
    case AddressErrorException: return   SIGSEGV;
    case     OverflowException: return    SIGFPE;
    case IllegalInstrException: return    SIGILL;
    case   BreakpointException: return   SIGTRAP;
    case           OutOfMemory: return SIGSTKFLT;
    default:                    return    SIGHUP;
    }
}

/*
 * Method: ComputeException
 * Description: This function translates a Unix signal into an exception.
 */
enum ExceptionType
GDBRemoteDebugger::ComputeException (int sig) {
  switch (sig)
    {
    case    SIGILL: return IllegalInstrException;
    case   SIGTRAP: return BreakpointException;
    case    SIGBUS: return BusErrorException;
    case    SIGFPE: return OverflowException;
    case   SIGSEGV: return AddressErrorException;
    case SIGSTKFLT: return OutOfMemory;
    default:        return NoException;
    }
}

/*
 * Method: HandleException
 * Description: This function handles the exception that has occurred by 
 *              first telling GDB which exception occurred.
 */
void
GDBRemoteDebugger::HandleException (int number) {
  register int sig;
  char *ptr;

  sig = ComputeSignal((enum ExceptionType)number);

  DEBUG( (char *)DB_RGDB , (char *)"Handling signal %d\n", sig);

  ptr = remcomOutBuffer;

  *ptr++ = 'T';

  // First output the signal number
  *ptr++ = hexchars[sig >> 4];
  *ptr++ = hexchars[sig & 0xf];

  // Now output register#:register_contents; for each register of interest
  *ptr++ = hexchars[GDBPCReg >> 4];
  *ptr++ = hexchars[GDBPCReg & 0xf];
  *ptr++ = ':';
  ptr = MemToHex((unsigned char *)&registers[GDBPCReg], ptr, 4);
  *ptr++ = ';';

  *ptr++ = hexchars[72 >> 4];
  *ptr++ = hexchars[72 & 0xf];
  *ptr++ = ':';
  ptr = MemToHex((unsigned char *)&registers[30], ptr, 4);
  *ptr++ = ';';

  *ptr++ = hexchars[StackReg >> 4];
  *ptr++ = hexchars[StackReg & 0xf];
  *ptr++ = ':';
  ptr = MemToHex((unsigned char *)&registers[StackReg], ptr, 4);
  *ptr++ = ';';

  // Finally, output thread:thread#; for each running thread
  for (struct nachos_thread *thread = allThreads.head; thread != NULL;
       thread = thread->next)
    {
      strcpy(ptr, "thread");
      ptr += 6;
      *ptr++ = ':';
      ptr = MemToHex((unsigned char *)&thread->ID, ptr, 4);
      *ptr++ = ';';
    }

  *ptr++ = '\0';

  PutPacket();

  Handshake(sig);
}

/*
 * Method: Handshake
 * Description: This function implements the "handshake" protocol between the
 *              GDB host and target.
 */
void
GDBRemoteDebugger::Handshake (int sig) {
  int value[BUFMAX / 4], addr, length, error, signl;
  char *ptr;
  struct nachos_thread *cthread = NULL, *gthread = NULL;
  BreakpointType type;

  for (;;)
    {
      error = 0;
      remcomOutBuffer[0] = '\0';

      GetPacket();

      DEBUG( (char *)DB_RGDB , (char *)"Got %s\n", remcomInBuffer);
      switch (remcomInBuffer[0])
	{
	case '?':	/* last signal */
	  remcomOutBuffer[0] = 'S';
	  remcomOutBuffer[1] = hexchars[sig >> 4];
	  remcomOutBuffer[2] = hexchars[sig & 0xf];
	  remcomOutBuffer[3] = '\0';
	  break;

	case 'c':	/* continue */
	  if (cthread != NULL && cthread->thread != currentThread)
	    currentThread->ForceYield(static_cast<Thread *>(cthread->thread));

	  ptr = &remcomInBuffer[1];

	  if (HexToInt(&ptr, &addr))
	    registers[GDBPCReg] = addr;

	  machine->singleStep = false;
	  ReturnFromException();
	  return;

	case 'C':	/* continue with signal */
	  if (cthread != NULL && cthread->thread != currentThread)
	    currentThread->ForceYield(static_cast<Thread *>(cthread->thread));

	  ptr = &remcomInBuffer[1];

	  if (HexToInt(&ptr, &signl))
	    {
	      if (*ptr == ';')
		{
		  ptr++;
		  if (HexToInt(&ptr, &addr))
		    registers[GDBPCReg] = addr;
		}
	    }

	  machine->singleStep = false;
	  ReturnFromException();
	  machine->RaiseException(ComputeException(signl),
				  registers[GDBPCReg]);
	  return;

	case 'D':	/* detach */
	  printf("Detaching from remote debugger...\n");

	  machine->singleStep = false;
	  ReturnFromException();

	  delete remoteDebugger;
	  remoteDebugger = NULL;

	  /* No response for this command */

	  return;

	case 'g':	/* read registers */
	  if (gthread != NULL && gthread->thread != currentThread)
	    currentThread->ForceYield(static_cast<Thread *>(gthread->thread));

	  MemToHex((unsigned char *)registers, remcomOutBuffer,
		   NUMREGS * sizeof(int));
	  break;

	case 'G':	/* write registers */
	  if (gthread != NULL && gthread->thread != currentThread)
	    currentThread->ForceYield(static_cast<Thread *>(gthread->thread));

	  HexToMem((unsigned char *)&remcomInBuffer[1], (char *)registers,
		   NUMREGS * sizeof(int));
	  strcpy(remcomOutBuffer, "OK");
	  break;

	case 'H':	/* set thread */
	  ptr = &remcomInBuffer[2];
	  strcpy(remcomOutBuffer, "E01");
 	  if (HexToInt(&ptr, &addr))
	    {
	      if (addr == -1)
		{
		  switch (remcomInBuffer[1])
		    {
		    case 'c':
		      cthread = NULL;
		      strcpy(remcomOutBuffer, "OK");
		      break;
		    case 'g':
		      gthread = NULL;
		      strcpy(remcomOutBuffer, "OK");
		      break;
		    default:
		      DEBUG( (char *)DB_RGDB , (char *)"Don't know thread type %c\n",
			    remcomInBuffer[1]);
		      break;
		    }
		}
	      else
		{
		  for (struct nachos_thread *thread = allThreads.head;
		       thread != NULL; thread = thread->next)
		    {
		      if (addr == thread->ID)
			{
			  switch (remcomInBuffer[1])
			    {
			    case 'c':
			      cthread = thread;
			      strcpy(remcomOutBuffer, "OK");
			      break;
			    case 'g':
			      gthread = thread;
			      strcpy(remcomOutBuffer, "OK");
			      break;
			    default:
			      DEBUG( (char *)DB_RGDB , (char *)"Don't know thread type %c\n",
				    remcomInBuffer[1]);
			      break;
			    }
			  break;
			}
		    }
		}
	    }
	  break;

	case 'k':	/* kill request */
	  if (gthread != NULL && gthread->thread != currentThread)
	    currentThread->ForceYield(static_cast<Thread *>(gthread->thread));

	  currentThread->Finish();
	  ASSERT(false);
	  break;

	case 'm':	/* read memory */
	  if (gthread != NULL && gthread->thread != currentThread)
	    currentThread->ForceYield(static_cast<Thread *>(gthread->thread));

	  ptr = &remcomInBuffer[1];

	  if (HexToInt(&ptr, &addr) && *ptr++ == ',' &&
	      HexToInt(&ptr, &length))
	    {
	      if (!ReadMemory(addr, length, value))
		strcpy(remcomOutBuffer, "E03");

	      MemToHex((unsigned char *)value, remcomOutBuffer, length);
	    }
	  else
	    {
	      strcpy(remcomOutBuffer, "E01");
	    }
	  break;

	case 'M':	/* write memory */
	  if (gthread != NULL && gthread->thread != currentThread)
	    currentThread->ForceYield(static_cast<Thread *>(gthread->thread));

	  ptr = &remcomInBuffer[1];

	  if (HexToInt(&ptr, &addr) && *(ptr++) == ',' &&
	      HexToInt(&ptr, &length) && *(ptr++) == ':')
	    {
	      HexToMem((unsigned char *)ptr, (char *)value, length);

	      if (WriteMemory(addr, length, value))
		{
		  strcpy(remcomOutBuffer, "OK");
		}
	      else
		{
		  strcpy(remcomOutBuffer, "E03");
		}
	    }
	  else
	    {
	      strcpy(remcomOutBuffer, "E02");
	    }
	  break;

	case 'p':	/* read register */
	  if (gthread != NULL && gthread->thread != currentThread)
	    currentThread->ForceYield(static_cast<Thread *>(gthread->thread));

	  ptr = &remcomInBuffer[1];
	  if (HexToInt(&ptr, &addr))
	    {
	      IntToHex(machine->ReadRegister(addr), remcomOutBuffer, 4);
	    }
	  else
	    {
	      strcpy(remcomOutBuffer, "E01");
	    }
	  break;

	case 'P':	/* write register */
	  if (gthread != NULL && gthread->thread != currentThread)
	    currentThread->ForceYield(static_cast<Thread *>(gthread->thread));

	  ptr = &remcomInBuffer[1];
	  if (HexToInt(&ptr, &addr))
	    {
	      if (*ptr == '=')
		{
		  ptr++;
		  if (HexToInt(&ptr, value))
		    {
		      machine->WriteRegister(addr, value[0]);
		      strcpy(remcomOutBuffer, "OK");
		      break;
		    }
		}
	    }

	  strcpy(remcomOutBuffer, "E01");
	  break;

	case 'q':	/* general query */
	  ProcessQuery();
	  break;
	  
	case 's':	/* step */
	  if (cthread != NULL && cthread->thread != currentThread)
	    currentThread->ForceYield(static_cast<Thread *>(cthread->thread));

	  ptr = &remcomInBuffer[1];

	  if (HexToInt(&ptr, &addr))
	    registers[GDBPCReg] = addr;

	  machine->singleStep = true;
	  ReturnFromException();
	  return;

	case 'S':	/* step with signal */
	  if (cthread != NULL && cthread->thread != currentThread)
	    currentThread->ForceYield(static_cast<Thread *>(cthread->thread));

	  ptr = &remcomInBuffer[1];

	  if (HexToInt(&ptr, &signl))
	    {
	      if (*ptr == ';')
		{
		  ptr++;
		  if (HexToInt(&ptr, &addr))
		    registers[GDBPCReg] = addr;
		}
	    }

	  machine->singleStep = true;
	  ReturnFromException();
	  machine->RaiseException(ComputeException(signl),
				  registers[GDBPCReg]);
	  return;

	case 'T':	/* thread alive */
	  ptr = &remcomInBuffer[1];
	  strcpy(remcomOutBuffer, "E01");
 	  if (HexToInt(&ptr, &addr))
	    {
	      for (struct nachos_thread *thread = allThreads.head;
		   thread != NULL; thread = thread->next)
		{
		  if (addr == thread->ID)
		    {
		      strcpy(remcomOutBuffer, "OK");
		      break;
		    }
		}
	    }

	  break;

	case 'z':	/* remove breakpoint/watchpoint */
	  type = static_cast<BreakpointType>(remcomInBuffer[1] - '0');
	  ptr = &remcomInBuffer[2];

	  if (*(ptr++) == ',' && HexToInt(&ptr, &addr) &&
	      *(ptr++) == ',' && HexToInt(&ptr, &length))
	    {
	      machine->breakpoints.removeBreakpoint(addr, length, type);
	    }
	  strcpy(remcomOutBuffer, "OK");
	  break;

	case 'Z':	/* insert breakpoint/watchpoint */
	  type = static_cast<BreakpointType>(remcomInBuffer[1] - '0');
	  ptr = &remcomInBuffer[2];

	  if (*(ptr++) == ',' && HexToInt(&ptr, &addr) &&
	      *(ptr++) == ',' && HexToInt(&ptr, &length))
	    {
	      machine->breakpoints.addBreakpoint(addr, length, type);
	    }
	  strcpy(remcomOutBuffer, "OK");
	  break;

	default:	/* unsupported command */
	  break;
	}
      PutPacket();
    }
}

/*
 * Method: ProcessQuery
 * Description: This function parses and responds to queries from the remote
 *              GDB, according to the general query protocol.
 */
void
GDBRemoteDebugger::ProcessQuery () {
  int value[BUFMAX / 4], addr, length;
  char *ptr;

  switch (remcomInBuffer[1])
    {
    case 'C':
      if (remcomInBuffer[2] == '\0')
	{
	  remcomOutBuffer[0] = 'Q';
	  remcomOutBuffer[1] = 'C';
	  IntToHex(currentThread->Get_Id(), &remcomOutBuffer[2], 2);
	}
      else if (remcomInBuffer[2] == 'R' && remcomInBuffer[3] == 'C' &&
	       remcomInBuffer[4] == ':')
	{
	  ptr = &remcomInBuffer[5];
	  if (HexToInt(&ptr, &addr) && *ptr++ == ',' &&
	      HexToInt(&ptr, &length))
	    {
	      if (!ReadMemory(addr, length, value))
		strcpy(remcomOutBuffer, "E03");

	      remcomOutBuffer[0] = 'C';
	      IntToHex(CRC32((unsigned char *) value, length, 0xffffffff),
		       &remcomOutBuffer[1], 4);
	    }
	  else
	    {
	      strcpy(remcomOutBuffer, "E01");
	    }
	}
      break;

    case 'O':
      if (strncmp(&remcomInBuffer[2], "ffsets", 6) == 0)
	{
	  strcpy(remcomOutBuffer, "Text=0000;Data=0000;Bss=0000");
	}
      break;

    default:
      break;
    }
}

/*
 * Method: ReturnFromException
 * Description: This function prepares to return from an exception by restoring
 *              the register state.
 */
void
GDBRemoteDebugger::ReturnFromException () {
  DEBUG( (char *)DB_RGDB , (char *)"Returning from exception\n");
  RestoreRegisters();
}

#endif

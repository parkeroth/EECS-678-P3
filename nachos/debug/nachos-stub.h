/*
 * File: nachos-stub.cc
 * Author: Sean B House, University of Kansas
 * Description: This is the Nachos (MIPS) GDB stub that facilitates
 *              debugging of Nachos user-level programs through GDB
 *              remote debugging.
 */

#ifndef __NACHOSSTUB_H__
#define __NACHOSSTUB_H__

#ifdef USER_PROGRAM

#pragma interface "debug/nachos-stub.h"

#include "machine.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

/******************************************************************************
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers.
 * At least NUMREGBYTES*2 are needed for register packets.
 *****************************************************************************/
#define BUFMAX 2048
#define NUMREGS 38

#define KILL_REMOTE_DEBUGGER -1

/* These are the register that differ between Nachos and GDB. */
#define GDBBadVAddrReg 35
#define GDBPCReg 37

class GDBRemoteDebugger {
public:
  GDBRemoteDebugger(int port);
  void Exit(void);
  void Initialize(void);
  void GDBCatchException(int number);
  int  PutDebugChar(char ch);
  char GetDebugChar(void);
  void HandleException(int number);
  void Handshake(int sigval);
  void ProcessQuery(void);
  void ReturnFromException(void);
  void SaveRegisters(void);
  void RestoreRegisters(void);
  unsigned long CRC32 (unsigned char *buf, int len, unsigned int crc);
  bool ReadMemory(int addr, int length, int *value);
  bool WriteMemory(int addr, int length, int *value);
  void PutPacket(void);
  void GetPacket(void);
  char *MemToHex (unsigned char *mem, char *buf, int count);
  char *HexToMem (unsigned char *mem, char *buf, int count);
  char *IntToHex (int num, char *buf, int count) {
    return MemToHex((unsigned char *)&num, buf, count);
  }
  int HexToInt(char **ptr, int *intValue);
  int ComputeSignal(enum ExceptionType exception);
  enum ExceptionType ComputeException(int signal);
  int Hex(char ch);
private:
  int sockFD;
  int newSockFD;
  struct sockaddr_in serverAddr;
  struct sockaddr_in clientAddr;
  char remcomInBuffer[BUFMAX];
  char remcomOutBuffer[BUFMAX];
  int registers[NUMREGS];
};

#endif /* USER_PROGRAM */
#endif /* __NACHOSSTUB_H__ */

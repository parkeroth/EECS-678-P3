// mipssim.h 
//	Internal data structures for simulating the MIPS instruction set.
//
//  DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef MIPSSIM_H
#define MIPSSIM_H

#pragma interface "machine/mipssim.h"

#include "copyright.h"

/* Include ifdef_symbols.h because we defined the flag 
 * REMOTE_USER_PROGRAM_DEBUGGING within that file
 */

#include "../threads/ifdef_symbols.h"

/*
 * OpCode values.  The names are straight from the MIPS
 * manual except for the following special ones:
 *
 * OP_UNIMP -		means that this instruction is legal, but hasn't
 *			been implemented in the simulator yet.
 * OP_RES -		means that this is a reserved opcode (it isn't
 *			supported by the architecture).
 */

#define OP_ADD		1
#define OP_ADDI		2
#define OP_ADDIU	3
#define OP_ADDU		4
#define OP_AND		5
#define OP_ANDI		6
#define OP_BEQ		7
#define OP_BGEZ		8
#define OP_BGEZAL	9
#define OP_BGTZ		10
#define OP_BLEZ		11
#define OP_BLTZ		12
#define OP_BLTZAL	13
#define OP_BNE		14
#ifdef REMOTE_USER_PROGRAM_DEBUGGING
#define OP_BREAK        15
#endif
#define OP_DIV		16
#define OP_DIVU		17
#define OP_J		18
#define OP_JAL		19
#define OP_JALR		20
#define OP_JR		21
#define OP_LB		22
#define OP_LBU		23
#define OP_LH		24
#define OP_LHU		25
#define OP_LUI		26
#define OP_LW		27
#define OP_LWL		28
#define OP_LWR		29

#define OP_MFHI		31
#define OP_MFLO		32

#define OP_MTHI		34
#define OP_MTLO		35
#define OP_MULT		36
#define OP_MULTU	37
#define OP_NOR		38
#define OP_OR		39
#define OP_ORI		40
#define OP_RFE		41
#define OP_SB		42
#define OP_SH		43
#define OP_SLL		44
#define OP_SLLV		45
#define OP_SLT		46
#define OP_SLTI		47
#define OP_SLTIU	48
#define OP_SLTU		49
#define OP_SRA		50
#define OP_SRAV		51
#define OP_SRL		52
#define OP_SRLV		53
#define OP_SUB		54
#define OP_SUBU		55
#define OP_SW		56
#define OP_SWL		57
#define OP_SWR		58
#define OP_XOR		59
#define OP_XORI		60
#define OP_SYSCALL	61
#define OP_UNIMP	62
#define OP_RES		63
#define MaxOpcode	63

/*
 * Miscellaneous definitions:
 */

/* Change a table index to an address offset assuming table elements of size 4
   bytes */
#define IndexToAddr(x) ((x) << 2)

#define SIGN_BIT	0x80000000
#define R31		31

/*
 * The table below is used to translate bits 31:26 of the instruction
 * into a value suitable for the "opCode" field of a MemWord structure,
 * or into a special value for further decoding.
 */

#define SPECIAL 100
#define BCOND	101

// See page A-3 of the R4000 User's Manual
#define IFMT 1	// Immediate operand
#define JFMT 2	// "Jump" operand
#define RFMT 3	// Register format

struct OpInfo {
    int opCode;		/* Translated op code. */
    int format;		/* Format type (IFMT or JFMT or RFMT) */
};

static OpInfo opTable[] = {
    {SPECIAL, RFMT}, {BCOND, IFMT}, {OP_J, JFMT}, {OP_JAL, JFMT},
    {OP_BEQ, IFMT}, {OP_BNE, IFMT}, {OP_BLEZ, IFMT}, {OP_BGTZ, IFMT},
    {OP_ADDI, IFMT}, {OP_ADDIU, IFMT}, {OP_SLTI, IFMT}, {OP_SLTIU, IFMT},
    {OP_ANDI, IFMT}, {OP_ORI, IFMT}, {OP_XORI, IFMT}, {OP_LUI, IFMT},
    {OP_UNIMP, IFMT}, {OP_UNIMP, IFMT}, {OP_UNIMP, IFMT}, {OP_UNIMP, IFMT},
    {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT},
    {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT},
    {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT},
    {OP_LB, IFMT}, {OP_LH, IFMT}, {OP_LWL, IFMT}, {OP_LW, IFMT},
    {OP_LBU, IFMT}, {OP_LHU, IFMT}, {OP_LWR, IFMT}, {OP_RES, IFMT},
    {OP_SB, IFMT}, {OP_SH, IFMT}, {OP_SWL, IFMT}, {OP_SW, IFMT},
    {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_SWR, IFMT}, {OP_RES, IFMT},
    {OP_UNIMP, IFMT}, {OP_UNIMP, IFMT}, {OP_UNIMP, IFMT}, {OP_UNIMP, IFMT},
    {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT},
    {OP_UNIMP, IFMT}, {OP_UNIMP, IFMT}, {OP_UNIMP, IFMT}, {OP_UNIMP, IFMT},
    {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT}
};

/*
 * The table below is used to convert the "funct" field of SPECIAL
 * instructions into the "opCode" field of a MemWord.
 */

static int specialTable[] = {
    OP_SLL, OP_RES, OP_SRL, OP_SRA, OP_SLLV, OP_RES, OP_SRLV, OP_SRAV,
#ifdef REMOTE_USER_PROGRAM_DEBUGGING
    OP_JR, OP_JALR, OP_RES, OP_RES, OP_SYSCALL, OP_BREAK, OP_RES, OP_RES,
#else
    OP_JR, OP_JALR, OP_RES, OP_RES, OP_SYSCALL, OP_UNIMP, OP_RES, OP_RES,
#endif
    OP_MFHI, OP_MTHI, OP_MFLO, OP_MTLO, OP_RES, OP_RES, OP_RES, OP_RES,
    OP_MULT, OP_MULTU, OP_DIV, OP_DIVU, OP_RES, OP_RES, OP_RES, OP_RES,
    OP_ADD, OP_ADDU, OP_SUB, OP_SUBU, OP_AND, OP_OR, OP_XOR, OP_NOR,
    OP_RES, OP_RES, OP_SLT, OP_SLTU, OP_RES, OP_RES, OP_RES, OP_RES,
    OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES,
    OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES
};


// Stuff to help print out each instruction, for debugging

/* RS = Source register
   RT = Target register (can be source or destination)
   RD = Destination register
   EXTRA = Anything besides a register (immediate, address, etc.)
*/
enum RegType { NONE, RS, RT, RD, EXTRA }; 

struct OpString {
    char *string;	// Printed version of instruction
    RegType args[3];
};

static struct OpString opStrings[] = {
	{(char *)"Shouldn't happen", {NONE, NONE, NONE}},
	{(char *)"ADD r%d,r%d,r%d", {RD, RS, RT}},
	{(char *)"ADDI r%d,r%d,%d", {RT, RS, EXTRA}},
	{(char *)"ADDIU r%d,r%d,%d", {RT, RS, EXTRA}},
	{(char *)"ADDU r%d,r%d,r%d", {RD, RS, RT}},
	{(char *)"AND r%d,r%d,r%d", {RD, RS, RT}},
	{(char *)"ANDI r%d,r%d,%d", {RT, RS, EXTRA}},
	{(char *)"BEQ r%d,r%d,%d", {RS, RT, EXTRA}},
	{(char *)"BGEZ r%d,%d", {RS, EXTRA, NONE}},
	{(char *)"BGEZAL r%d,%d", {RS, EXTRA, NONE}},
	{(char *)"BGTZ r%d,%d", {RS, EXTRA, NONE}},
	{(char *)"BLEZ r%d,%d", {RS, EXTRA, NONE}},
	{(char *)"BLTZ r%d,%d", {RS, EXTRA, NONE}},
	{(char *)"BLTZAL r%d,%d", {RS, EXTRA, NONE}},
	{(char *)"BNE r%d,r%d,%d", {RS, RT, EXTRA}},
#ifdef REMOTE_USER_PROGRAM_DEBUGGING
	{(char *)"BREAK", {NONE, NONE, NONE}},
#else
	{(char *)"Shouldn't happen", {NONE, NONE, NONE}},
#endif
	{(char *)"DIV r%d,r%d", {RS, RT, NONE}},
	{(char *)"DIVU r%d,r%d", {RS, RT, NONE}},
	{(char *)"J %d", {EXTRA, NONE, NONE}},
	{(char *)"JAL %d", {EXTRA, NONE, NONE}},
	{(char *)"JALR r%d,r%d", {RD, RS, NONE}},
	{(char *)"JR r%d,r%d", {RD, RS, NONE}},
	{(char *)"LB r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char *)"LBU r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char *)"LH r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char *)"LHU r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char *)"LUI r%d,%d", {RT, EXTRA, NONE}},
	{(char *)"LW r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char *)"LWL r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char *)"LWR r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char *)"Shouldn't happen", {NONE, NONE, NONE}},
	{(char *)"MFHI r%d", {RD, NONE, NONE}},
	{(char *)"MFLO r%d", {RD, NONE, NONE}},
	{(char *)"Shouldn't happen", {NONE, NONE, NONE}},
	{(char *)"MTHI r%d", {RS, NONE, NONE}},
	{(char *)"MTLO r%d", {RS, NONE, NONE}},
	{(char *)"MULT r%d,r%d", {RS, RT, NONE}},
	{(char *)"MULTU r%d,r%d", {RS, RT, NONE}},
	{(char *)"NOR r%d,r%d,r%d", {RD, RS, RT}},
	{(char *)"OR r%d,r%d,r%d", {RD, RS, RT}},
	{(char *)"ORI r%d,r%d,%d", {RT, RS, EXTRA}},
	{(char *)"RFE", {NONE, NONE, NONE}},
	{(char *)"SB r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char *)"SH r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char *)"SLL r%d,r%d,%d", {RD, RT, EXTRA}},
	{(char *)"SLLV r%d,r%d,r%d", {RD, RT, RS}},
	{(char *)"SLT r%d,r%d,r%d", {RD, RS, RT}},
	{(char *)"SLTI r%d,r%d,%d", {RT, RS, EXTRA}},
	{(char *)"SLTIU r%d,r%d,%d", {RT, RS, EXTRA}},
	{(char *)"SLTU r%d,r%d,r%d", {RD, RS, RT}},
	{(char *)"SRA r%d,r%d,%d", {RD, RT, EXTRA}},
	{(char *)"SRAV r%d,r%d,r%d", {RD, RT, RS}},
	{(char *)"SRL r%d,r%d,%d", {RD, RT, EXTRA}},
	{(char *)"SRLV r%d,r%d,r%d", {RD, RT, RS}},
	{(char *)"SUB r%d,r%d,r%d", {RD, RS, RT}},
	{(char *)"SUBU r%d,r%d,r%d", {RD, RS, RT}},
	{(char *)"SW r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char *)"SWL r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char *)"SWR r%d,%d(r%d)", {RT, EXTRA, RS}},
	{(char *)"XOR r%d,r%d,r%d", {RD, RS, RT}},
	{(char *)"XORI r%d,r%d,%d", {RT, RS, EXTRA}},
	{(char *)"SYSCALL", {NONE, NONE, NONE}},
	{(char *)"Unimplemented", {NONE, NONE, NONE}},
	{(char *)"Reserved", {NONE, NONE, NONE}}
      };

#endif // MIPSSIM_H

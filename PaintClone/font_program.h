#pragma once

#include "common.h"
#include "custom_types.h"
#include "dynamic_array.h"
#include "stack.h"

enum class TTFInstruction
{
	SPVTCA = 0x02,
	SPVTCA_MAX = 0x03,
	FDEF = 0x2C,
	ENDF = 0x2D,
	NPUSHB = 0x40,
	NPUSHW = 0x41,
	PUSHB = 0xB0,
	PUSHB_MAX = 0xB7,
	PUSHW = 0xB8,
	PUSHW_MAX = 0xBF,
};

struct TTFProgramFunction
{
	ubyte* functionAddress; // refers to original instructions buffer
	int functionLength;
};

struct TTFInstructions
{
	ubyte* instructions;
	int length;
};

struct TTFGraphicsState
{
	bool autoFlip;
	int controlValueCutIn;
	int deltaBase;
	int deltaShift;
	int2 dualProjectionVectors;
	int2 freedomVector;
	int zp0;
	int zp1;
	int zp2;
	int instructControl;
	uint loop;
	int minimum_distance;
	int2 projectionVector;
	int2 roundState;
	int rp0;
	int rp1;
	int rp2;
	int scanControl;
	int singeWidthCutIn;
	int singleWidthValue;
};

struct TTFProgram
{
	TTFInstructions fpgmInstructions; // executed when font is first loaded
	TTFInstructions prepInstructions; // executed when font size is changed
	//TTFInstructions glyphInstructions;

	ubyte* instructions;
	int instructionsLength;
	ubyte* currentInstruction;

	int executedInstructions; // for debug

	SimpleDynamicArray<int>* cv;
	SimpleDynamicArray<TTFProgramFunction>* functions;
	Stack<uint>* stack;

	TTFGraphicsState graphicsState;
};

//#define TTY_FIX_MUL(a, b, addend, shift) TTY_ROUNDED_DIV_POW2((TTY_S64)(a) * (TTY_S64)(b), addend, shift)

//#define TTY_F2DOT30_MUL(a, b) FIX_MUL(a, b, 0x20000000, 30)

void TTFExecuteProgram(TTFProgram program);

void TTFExecuteSPVTCA(TTFProgram* program);

void TTFExecuteFDEF(TTFProgram* program);

void TTFExecuteNPUSHB(TTFProgram* program);
void TTFExecuteNPUSHW(TTFProgram* program);

void TTFExecutePUSHB(TTFProgram* program);
void TTFExecutePUSHW(TTFProgram* program);
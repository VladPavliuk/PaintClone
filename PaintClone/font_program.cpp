#include "font_program.h"

//TODO: it would be better to split it into seperate functions for different programs (fpgm, prep, glypInst)
void TTFExecuteProgram(TTFProgram program)
{
	program.executedInstructions = 0;
	ubyte* lastInstructionAddress = program.instructions + program.instructionsLength;

	while ((uint)program.currentInstruction < (uint)lastInstructionAddress)
	{
		program.executedInstructions++;
		ubyte instruction = *program.currentInstruction;
		ConsolePrint("%i) inst op: %i\n", program.executedInstructions, instruction);
		program.currentInstruction++;

		if (instruction == (ubyte)TTFInstruction::FDEF)
		{
			TTFExecuteFDEF(&program);
		}
		else if (instruction == (ubyte)TTFInstruction::NPUSHB)
		{
			TTFExecuteNPUSHB(&program);
		}
		else if (instruction == (ubyte)TTFInstruction::NPUSHW)
		{
			TTFExecuteNPUSHW(&program);
		}
		else if ((ubyte)TTFInstruction::PUSHB <= instruction && instruction <= (ubyte)TTFInstruction::PUSHB_MAX)
		{
			TTFExecutePUSHB(&program);
		}
		else if ((ubyte)TTFInstruction::PUSHW <= instruction && instruction <= (ubyte)TTFInstruction::PUSHW_MAX)
		{
			TTFExecutePUSHW(&program);
		}
		else if ((ubyte)TTFInstruction::SPVTCA <= instruction && instruction <= (ubyte)TTFInstruction::SPVTCA_MAX)
		{
			TTFExecuteSPVTCA(&program);
		}
		else
		{
			assert(false);
		}
		//printf("d", test);
	}
}

void TTFExecuteFDEF(TTFProgram* program)
{
	int functionId = program->stack->pop();

	TTFProgramFunction function;
	function.functionAddress = program->currentInstruction;
	function.functionLength = 1;

	program->currentInstruction++;

	while (*program->currentInstruction != (ubyte)TTFInstruction::ENDF)
	{
		function.functionLength++;
		program->currentInstruction++;
	}

	program->functions->set(functionId, function);
	program->currentInstruction++;
}

void TTFPushOnStackByte(TTFProgram* program, int countToPush)
{
	for (int i = 0; i < countToPush; i++)
	{
		program->currentInstruction++;

		program->stack->enqueue(*program->currentInstruction);
	}
}

void TTFPushOnStackWord(TTFProgram* program, int countToPush)
{
	program->currentInstruction++;

	for (int i = 0; i < countToPush; i++)
	{
		// NOTE: instructions are stored in big endian as well
		short wordToPush = _byteswap_ushort(*(short*)program->currentInstruction);

		program->currentInstruction += 2;
		program->stack->enqueue(wordToPush);
	}
}

void TTFExecuteSPVTCA(TTFProgram* program)
{
	program->currentInstruction--;

	bool isXAxis = *program->currentInstruction & 0b00000001;

	if (isXAxis)
	{
		program->graphicsState.projectionVector.x = 0x4000;
		program->graphicsState.projectionVector.y = 0;
	}
	else
	{
		program->graphicsState.projectionVector.x = 0;
		program->graphicsState.projectionVector.y = 0x4000;
	}
	program->graphicsState.dualProjectionVectors = program->graphicsState.projectionVector;
	program->currentInstruction++;
}

void TTFExecuteNPUSHB(TTFProgram* program)
{
	int countToPush = (int)*program->currentInstruction;
	TTFPushOnStackByte(program, countToPush);
	program->currentInstruction++;
}

void TTFExecuteNPUSHW(TTFProgram* program)
{
	int countToPush = (int)*program->currentInstruction;
	TTFPushOnStackWord(program, countToPush);
	//program->currentInstruction++;
}

void TTFExecutePUSHB(TTFProgram* program)
{
	program->currentInstruction--;
	ubyte countToPush = 1 + ((ubyte)*program->currentInstruction & 0b00000111);
	TTFPushOnStackByte(program, (int)countToPush);
	program->currentInstruction++;
}

void TTFExecutePUSHW(TTFProgram* program)
{
	program->currentInstruction--;
	ubyte countToPush = 1 + ((ubyte)*program->currentInstruction & 0b00000111);
	TTFPushOnStackWord(program, (int)countToPush);
	//program->currentInstruction++;
}


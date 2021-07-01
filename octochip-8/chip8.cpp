#include <stdio.h>
#include <string.h>
//#include <Windows.h>
#include "chip8.h"

//Returns the registers and stack
void chip8::getRegisters(short values[]) {
	memcpy(values, V, sizeof(V));
	memcpy(&values[16], stack, sizeof(stack));
	values[32] = opcode;
	values[33] = pc;
	values[34] = I;
	values[35] = sp;
	values[38] = delay_timer;
	values[39] = sound_timer;
}
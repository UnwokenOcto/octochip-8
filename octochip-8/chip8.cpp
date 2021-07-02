#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <Windows.h>
#include "chip8.h"

//Fonstset
unsigned char chip8_fontset[80] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

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

//Initialize data
void chip8::init() {
	opcode = 0;
	I = 0;
	pc = 0x200;
	delay_timer = 0;
	sound_timer = 0;
	sp = 0;
	drawFlag = true;

	for (int i = 0; i < 4096; ++i) {
		memory[i] = 0;
	}
	for (int i = 0; i < 16; ++i) {
		V[i] = key[i] = 0;
	}
	for (int i = 0; i < 16; ++i) {
		stack[i] = 0;
	}
	for (int i = 0; i < 2048; ++i) {
		gfx[i] = 0;
	}

	//Load fontset into memory
	for (int i = 0; i < 80; ++i) {
		memory[i] = chip8_fontset[i];
	}
}

//Load application from file
bool chip8::loadApplication(const char* filename) {
	init();
	printf("Loading file: %s\n", filename);

	#pragma warning(suppress : 4996)
	FILE* romFile = fopen(filename, "rb");
	if (romFile == NULL) {
		fputs("Could not open file", stderr);
		return false;
	}

	//Get file size
	fseek(romFile, 0, SEEK_END);
	long romSize = ftell(romFile);
	rewind(romFile);

	//Create a buffer to hold the file
	char* buffer = (char*)malloc(sizeof(char) * romSize);
	if (buffer == NULL) {
		fputs("Error creating buffer to load file", stderr);
		return false;
	}

	//Copy file into buffer
	#pragma warning(suppress : 6386)
	size_t result = fread(buffer, 1, romSize, romFile);
	if (result != romSize) {
		fputs("Error copying file into buffer", stderr);
		return false;
	}

	//Copy ROM into memory, if it fits
	if ((4096 - 512) > romSize) { // ???? Should this be >= ????
		for (int i = 0; i < romSize; ++i) {
			#pragma warning(suppress : 6385)
			memory[i + 0x200] = buffer[i];
		}
	} else {
		fputs("File too big for memory", stderr);
		printf("Error: File too big to fit in memory.\n");
	}

	fclose(romFile);
	free(buffer);
	return true;
}
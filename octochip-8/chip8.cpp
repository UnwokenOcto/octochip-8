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
void chip8::getRegisters(unsigned short values[]) {
	int i = 0;
	for (; i < 16; ++i) { values[i] = V[i]; }
	for (; i < 32; ++i) { values[i] = stack[i-16]; }
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
	draw_flag = true;

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
	rom_size = ftell(romFile);
	rewind(romFile);

	//Create a buffer to hold the file
	char* buffer = (char*)malloc(sizeof(char) * rom_size);
	if (buffer == NULL) {
		fputs("Error creating buffer to load file", stderr);
		return false;
	}

	//Copy file into buffer
	#pragma warning(suppress : 6386)
	size_t result = fread(buffer, 1, rom_size, romFile);
	if (result != rom_size) {
		fputs("Error copying file into buffer", stderr);
		return false;
	}

	//Copy ROM into memory, if it fits
	if ((4096 - 512) > rom_size) { // ???? Should this be >= ????
		for (unsigned long i = 0; i < rom_size; ++i) {
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

//Emulate one CPU cycle
bool chip8::emulateCycle() {
	bool success = true;
	opcode = memory[pc] << 8 | memory[pc + 1];

	//printf("\n0x%03X    %04X    ", pc, opcode);
	//Execute opcode
	switch (opcode & 0xF000) {
	case 0x0000:
		switch (opcode & 0x00FF) {
		case 0x00E0: //00E0: Clears the screen
			for (int i = 0; i < 2048; ++i) {
				gfx[i] = 0;
			}
			draw_flag = true;
			pc += 2;
			break;

		case 0x00EE: //00EE: Returns from a subroutine
			--sp;
			pc = stack[sp];
			stack[sp] = 0;
			pc += 2;
			break;

		default: //0NNN: Unnecessary
			printf("Unknown opcode: 0x%X\n", opcode); //???? Increment pc by 2 ????
			break;
		}

	case 0x1000: //1NNN: Jumps to address NNN
		pc = opcode & 0x0FFF;
		break;

	case 0x2000: //2NNN: Calls subroutine at NNN
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		break;

	case 0x3000: //3XNN: Skips next instruction if VX == NN
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
			pc += 4;
		} else {
			pc += 2;
		}
		break;

	case 0x4000: //4XNN: Skips next instruction if VX != NN
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
			pc += 4;
		} else {
			pc += 2;
		}
		break;

	case 0x5000: //5XY0: Skips next instruction if VX == VY
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
			pc += 4;
		} else {
			pc += 2;
		}
		break;

	case 0x6000: //6XNN: Sets VX to NN
		V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
		pc += 2;
		break;

	case 0x7000: //7XNN: Adds NN to VX (Carry flag not changed)
		V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
		pc += 2;
		break;

	case 0x8000:
		switch (opcode & 0x000F) {
		case 0x0004: //8XY4: Adds VY to VX (Set VF to 1 if carry)
			if (V[(opcode & 0x00F0) >> 4] & (0xFF - V[(opcode & 0x0F00) >> 8])) {
				V[0xF] = 1;
			} else {
				V[0xF] = 0;
			}
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		}

	case 0xA000: //ANNN: Sets I to address NNN
		I = opcode & 0x0FFF;
		pc += 2;
		break;

	case 0xD000: { //DXYN: Draws sprite at coordinate (VX, VY) with width of 8 and height of N+1 pixels
		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;
		V[0xF] = 0;

		for (int yline = 0; yline < height; ++yline) {
			pixel = memory[I + yline];
			for (int xline = 0; xline < 8; ++xline) {
				if ((pixel & (0x80 >> xline)) != 0) {
					if (gfx[x + xline + ((y + yline) * 64)] == 1) {
						V[0xF] = 1;
					}
					gfx[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}
		
		/*for (int row = 0, mem_offset = 0; row < (opcode & 0x000F) + 1; ++row) {
			int location = V[(opcode & 0x0F00) >> 8] + (V[(opcode & 0x00F0) >> 4] * 64);
			gfx[location] = memory[I + mem_offset++];
		}*/

		draw_flag = true;
		pc += 2;
		}
		break;

	case 0xF000:
		switch (opcode & 0x00FF) {
		case 0x33: //FX33: Stores binary-coded decimal of VX at I
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
			break;
		}

	default:
		printf("\n\nPC: %04X\nOP: %04X", pc, opcode);
		success = false;
		break;
	}

	//Update timers
	if (delay_timer > 0) {
		--delay_timer;
	}
	if (sound_timer > 0) {
		if (sound_timer == 1) {
			printf("BEEP!\n\a");
		}
		--sound_timer;
	}

	return success;
}
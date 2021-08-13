#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <3ds.h>
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

	//Initialize random generator
	srand((int)time(NULL));
}

//Load application from file
bool chip8::loadApplication(const char* filename) {
	init();
	printf("\x1b[25;1HLoading file: %s                                \n", filename);

	Handle fileHandle;
	FS_Path archPath = { PATH_EMPTY, 1, "" };
	FS_Path filePath;
	u32 bytesRead;
	filePath = fsMakePath(PATH_ASCII, filename);
	unsigned char buffer[4096 - 512];

	//Open file
	FSUSER_OpenFileDirectly(&fileHandle, ARCHIVE_SDMC, archPath, filePath, FS_OPEN_READ, 0);

	//Get file size
	FSFILE_GetSize(fileHandle, &rom_size);
	if (rom_size == 0) {
		return false;
	}

	//Copy file into buffer
	FSFILE_Read(fileHandle, &bytesRead, 0, &buffer, sizeof(buffer));
	FSFILE_Close(fileHandle);

	//Copy ROM into memory, if it fits
	if ((4096 - 512) > rom_size) { // ???? Should this be >= ????
		for (unsigned long i = 0; i < rom_size; ++i) {
			memory[i + 0x200] = buffer[i];
		}
	} else {
		fputs("File too big for memory", stderr);
		printf("Error: File too big to fit in memory.\n");
		return false;
	}

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
			printf("\x1b[25;1H\x1b[31mUnknown opcode %04X found\nat location %04X\x1b[0m\x1b[0m", opcode, pc);
			success = false;
			break;
		} break;

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
		case 0x0000: //8XY0: Sets VX to the value of VY
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0001: //8XY1: Sets VX to VX OR VY
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0002: //8XY2: Sets VX to VX AND VY
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0003: //8XY3: Sets VX to VX XOR VY
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0004: //8XY4: Adds VY to VX (Set VF to 1 when there is a carry)
			if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) {
				V[0xF] = 1;
			} else {
				V[0xF] = 0;
			}
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0005: //8XY5: Sets VX to VX - VY (Set VF to 1 when there is no borrow)
			if (V[(opcode & 0x00F0) >> 4] > (V[(opcode & 0x0F00) >> 8])) {
				V[0xF] = 0;
			} else {
				V[0xF] = 1;
			}
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0006: //8XY6: Stores the least significant bit of VX in VF then shifts VX right by 1
			V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x01;
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;

		case 0x0007: //8XY7: Sets VX to VY - VX (Set VF to 1 when there is no borrow)
			if (V[(opcode & 0x0F00) >> 8] > (V[(opcode & 0x00F0) >> 4])) {
				V[0xF] = 0;
			} else {
				V[0xF] = 1;
			}
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x000E: //8XYE: Stores the most significant bit of VX in VF then shifts VX to the left by 1
			V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
			V[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2;
			break;

		default:
			printf("\x1b[25;1H\x1b[31mUnknown opcode %04X found\nat location %04X\x1b[0m\x1b[0m", opcode, pc);
			success = false;
			break;
		} break;

	case 0x9000: //9XY0: Skips next instruction if VX != VY
		if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
			pc += 4;
		} else {
			pc += 2;
		}
		break;

	case 0xA000: //ANNN: Sets I to address NNN
		I = opcode & 0x0FFF;
		pc += 2;
		break;

	case 0xB000: //BNNN: Jumps to address NNN + V0
		pc = (opcode & 0x0FFF) + V[0x0];
		break;

	case 0xC000: //CXNN: Sets VX to the result of bitwise AND on a random number (0-255) and NN
		V[(opcode & 0x0F00) >> 8] = (rand() % 255) & (opcode & 0x00FF);
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

		draw_flag = true;
		pc += 2;
		} break;
		
	case 0xE000:
		switch (opcode & 0x00FF) {
		case 0x009E: //EX9E: Skips next instruction if the key stored in VX is pressed
			if (key[V[(opcode & 0x0F00) >> 8]] == 1) {
				pc += 4;
			} else {
				pc += 2;
			}
			break;

		case 0x00A1: //EXA1: Skips next instruction if the key stored in VX is not pressed
			if (key[V[(opcode & 0x0F00) >> 8]] == 0) {
				pc += 4;
			} else {
				pc += 2;
			}
			break;

		default:
			printf("\x1b[25;1H\x1b[31mUnknown opcode %04X found\nat location %04X\x1b[0m\x1b[0m", opcode, pc);
			success = false;
			break;
		} break;

	case 0xF000:
		switch (opcode & 0x00FF) {
		case 0x0007: //FX07: Sets VX to the value of the delay timer
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

		case 0x000A: //FX0A: A key press is awaited, then stored in VX
			for (int i = 0; i <= 0xF; ++i) {
				if (key[i] == 1) {
					pc += 2;
				}
			} break;

		case 0x0015: //FX15: Sets the delay timer to VX
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0018: //FX18: Sets the sound timer to VX
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x001E: //FX1E: Adds VX to I (VF not affected)
			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0029: //FX29: Sets I to the location of the sprite for the character VX from the fontset
			I = V[(opcode & 0x0F00) >> 8] * 5;
			pc += 2;
			break;

		case 0x0033: //FX33: Stores binary-coded decimal of VX at I
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
			pc += 2;
			break;

		case 0x0055: //FX55: Stores V0 to VX (Including VX) in memory starting from address I
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
				memory[I + i] = V[i];
				//???? INCREMENTING MAY BE NECESSARY ????
			}
			pc += 2;
			break;

		case 0x0065: //FX65: Fills V0 to VX (Including VX) with values from memory starting from I
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
				V[i] = memory[I + i];
			}
			//???? INCREMENTING MAY BE NECESSARY ????
			pc += 2;
			break;

		default:
			printf("\x1b[25;1H\x1b[31mUnknown opcode %04X found\nat location %04X\x1b[0m\x1b[0m", opcode, pc);
			success = false;
			break;
		} break;

	default:
		printf("\x1b[25;1H\x1b[31mUnknown opcode %04X found\nat location %04X\x1b[0m\x1b[0m", opcode, pc);
		success = false;
		break;
	}

	//Update timers
	if (delay_timer > 0) {
		--delay_timer;
	}
	if (sound_timer > 0) {
		if (sound_timer == 1) {
			printf("\nBEEP!\a"); //Yes, I'm this lazy
		}
		--sound_timer;
	}

	return success;
}
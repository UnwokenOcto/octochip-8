#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
	printf("Chip-8 Disassembler\n");

	//Declare vars
	FILE* rom_file;
	unsigned short opcode;
	unsigned char memory[4096] = { 0 };
	unsigned short pc = 0x200; //Should this be 0x0 or 0x200?

	//Load file
	if (argv[1] == NULL) {
		printf("Please enter the file path of the Chip-8 ROM as an argument.");
		return(1);
	}
	printf("File: %s\n", argv[1]);
	#pragma warning(suppress : 4996)
	rom_file = fopen(argv[1], "rb");
	fread(&memory, 1, 4096, rom_file);
	fclose(rom_file);

	//Disassemble
	for (pc; pc < 4095; pc += 2) {
		opcode = (memory[pc] << 8) | memory[pc + 1];

		printf("\n0x%03X    %04X    ", pc, opcode);

		switch (opcode & 0xF000) {
		case 0x0000:
			switch (opcode & 0x00FF) {
			case 0x00E0: printf("Clears the screen"); break;
			case 0x00EE: printf("Returns from a subroutine"); break;
			} break;
		case 0x1000: printf("Jumps to address 0x%03X", opcode & 0x0FFF); break;
		case 0x2000: printf("Calls subroutine at 0x%03X", opcode & 0x0FFF); break;
		case 0x3000: printf("Skips next instruction if V%X == %02X", (opcode & 0x0F00) >> 8, opcode & 0x00FF); break;
		case 0x4000: printf("Skips next instruction if V%X != %02X", (opcode & 0x0F00) >> 8, opcode & 0x00FF); break;
		case 0x5000: printf("Skips next instruction if V%X == V%X", (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4); break;
		case 0x6000: printf("V%X = %02X", (opcode & 0x0F00) >> 8, opcode & 0x00FF); break;
		case 0x7000: printf("V%X += %02X (Carry flag not changed)", (opcode & 0x0F00) >> 8, opcode & 0x00FF); break;
		case 0x8000: 
			switch (opcode & 0x000F) {
			case 0x0000: printf("V%X = V%X", (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4); break;
			case 0x0001: printf("V%X = V%X | V%X (or)", (opcode & 0x0F00) >> 8, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4); break;
			case 0x0002: printf("V%X = V%X & V%X (and)", (opcode & 0x0F00) >> 8, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4); break;
			case 0x0003: printf("V%X = V%X ^ V%X (xor)", (opcode & 0x0F00) >> 8, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4); break;
			case 0x0004: printf("V%X += V%X (VF = 1 if there is a carry)", (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4); break;
			case 0x0005: printf("V%X -= V%X (VF = 1 if there is a borrow)", (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4); break;
			case 0x0006: printf("Stores LSB of V%X in VF, shifts V%X to the right by 1 (Diff implementations)", (opcode & 0x0F00) >> 8, (opcode & 0x0F00) >> 8); break;
			case 0x0007: printf("V%X = V%X - V%X (VF = 1 if there is a borrow)", (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4, (opcode & 0x0F00) >> 8); break;
			case 0x000E: printf("Stores MSB of V%X in VF, shifts V%X to the left by 1 (Diff implementations)", (opcode & 0x0F00) >> 8, (opcode & 0x0F00) >> 8); break;
			} break;
		case 0x9000: printf("Skips next instruction if V%X != V%X", (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4); break;
		case 0xA000: printf("I = %03X (Memory location)", opcode & 0x0FFF); break;
		case 0xB000: printf("Jumps to address 0x%03X + V0", opcode & 0x0FFF); break;
		case 0xC000: printf("V%X = (Random number) & %03X ", (opcode & 0x0F00) >> 8, opcode & 0x00FF); break;
		case 0xD000: printf("Draw sprite at (V%X, V%X) with a width/height of 8/(%X+1) pixels", (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4, opcode & 0x000F); break;
		case 0xE000:
			switch (opcode & 0x00FF) {
			case 0x009E: printf("Skips next instruction if key stored in V%X is pressed", (opcode & 0x0F00) >> 8); break;
			case 0x00A1: printf("Skips next instruction if key stored in V%X is not pressed", (opcode & 0x0F00) >> 8); break;
			} break;
		case 0xF000:
			switch (opcode & 0x00FF) {
			case 0x0007: printf("Sets V%X to the value of the delay timer", (opcode & 0x0F00) >> 8); break;
			case 0x000A: printf("Halts instruction until a keypress, and stores the key in V%X", (opcode & 0x0F00) >> 8); break;
			case 0x0015: printf("Sets the delay timer to V%X", (opcode & 0x0F00) >> 8); break;
			case 0x0018: printf("Sets the sound timer to V%X", (opcode & 0x0F00) >> 8); break;
			case 0x001E: printf("I += V%X (Does not affect VF)", (opcode & 0x0F00) >> 8); break;
			case 0x0029: printf("Sets I to the location the sprite for the character in V%X", (opcode & 0x0F00) >> 8); break;
			case 0x0033: printf("Stores the binary-coded decimal representation of V%X at I", (opcode & 0x0F00) >> 8); break;
			case 0x0055: printf("Stores the values from V0 to V%X starting at the memory address stored in I", (opcode & 0x0F00) >> 8); break;
			case 0x0065: printf("Fills the values from V0 to V%X starting at the memory address stored in I", (opcode & 0x0F00) >> 8); break;
			} break;

		//default: printf("Unknown opcode"); break;
		}
	}


	return 0;
}
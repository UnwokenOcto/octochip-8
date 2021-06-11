#include <stdio.h>
#include <Windows.h>
#include "chip8.h"

int temp() {
	printf("OctoChip-8");

	//Declare variables
	unsigned short opcode; //Current opcode
	unsigned char memory[4096]; //Memory
	unsigned char V[16]; //CPU registers
	unsigned short I; //Index register
	unsigned short pc; //Program counter
	unsigned char gfx[64 * 32]; //Pixels on the screen
	unsigned char delay_timer; //Both timers count at 60hz
	unsigned char sound_timer; //Sounds buzzer when 0 is reached
	unsigned short stack[16]; //Stack
	unsigned short sp; //Stack pointer
	unsigned char key[16]; //Current state of key inputs


	return 0;
}
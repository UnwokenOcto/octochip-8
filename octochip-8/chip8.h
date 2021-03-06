#pragma once

class chip8 {
public:
	bool draw_flag; //True whenever gfx has changed and screen needs to be updated
	unsigned char gfx[64 * 32]; //Pixels on the screen
	unsigned char key[16]; //Current state of key inputs

	bool emulateCycle(); //Emulate one CPU cycle
	bool loadApplication(const char* filename); //Load application from file
	void getRegisters(unsigned short values[]); //Returns the registers and stack

private:
	unsigned short opcode; //Current opcode
	unsigned char memory[4096]; //Memory
	unsigned char V[16]; //CPU registers
	unsigned short I; //Index register
	unsigned short pc; //Program counter
	unsigned char delay_timer; //Both timers count at 60hz
	unsigned char sound_timer; //Sounds buzzer when 0 is reached
	unsigned short stack[16]; //Stack
	unsigned short sp; //Stack pointer
	unsigned long rom_size; //ROM size

	void init(); //Initialize data
};
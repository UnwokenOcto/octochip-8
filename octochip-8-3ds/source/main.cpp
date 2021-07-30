#include <stdio.h>
#include <string>
#include <3ds.h>
#include <map>
#include "chip8.h"

class display3ds {
public:
	display3ds();
	~display3ds();
};

//Clears the console of the specified screen
void clearConsole(PrintConsole console) {
	consoleSelect(&console);
	printf("\x1b[30;1H");
	for (int i = 0; i < 30; ++i) {
		printf("\n");
	}
}

//Declare 3ds variables
PrintConsole topScr, botScr;

//Declare chip8 variables
chip8 myChip8; //The one and only
std::map<int, int> keymap = { //The keymap
	{ KEY_UP, 0x1 },
	{ KEY_DOWN, 0x4 },
	{ KEY_X, 0xC },
	{ KEY_B, 0xD }
};

//Main
int main(int argc, char* argv[]) {
	//Initialize display
	gfxInitDefault();
	consoleInit(GFX_TOP, &topScr);
	consoleInit(GFX_BOTTOM, &botScr);

	//TODO: load ROM

	//Control list
	char control_list[] =
		"OctoChip-8\n\n"
		"A    : Run normally\n"
		"Y    : Run one cycle\n"
		"Touch: Toggle Registers\n"
		"L/R  : Change speed by 50\n";
	consoleSelect(&botScr);
	printf("%s", control_list);

	//Main loop
	bool quit = false; //Quit flag
	int mode = 1; //Regular vs Cycle by cycle, see Modes comment below
	int cycles = 0; //Used for counting how many cycles actually execute per second
	bool display_registers = true; //Whether the registers should be displayed
	int max_cycles = 500; //Maximum cycles per second
	//Modes:
	//0 - Run normally
	//1 - Don't run cycle until Y is pressed
	//2 - Y has been pressed, run one cycle
	//3 - Unknown opcode, press start to quit
	double cycle_length = 1000.0 / max_cycles; //Ticks per cycle
	while (aptMainLoop() && !quit) {
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
		hidScanInput();

		//Execute button presses
		u32 kDown = hidKeysDown();
		if (kDown != 0) {
			if (kDown & KEY_START) { //Quit Program
				quit = true;
			}
			if (kDown & KEY_A) { //Run normally
				mode = 0;
			}
			if (kDown & KEY_Y) { //Y has been pressed, run one cycle
				mode = 2;
			}
			if (kDown & KEY_TOUCH) { //Toggle register display
				display_registers = !display_registers;
				if (display_registers) {
					consoleSelect(&botScr);
					printf("\x1b[1;1H%s", control_list);
				} else {
					clearConsole(botScr);
				}
			}
			if (kDown & KEY_R) { //Increase max speed by 50
				max_cycles += 50;
				cycle_length = 1000.0 / max_cycles;
			}
			if (kDown & KEY_L) { //Decrease max speed by 50
				if (max_cycles > 50) {
					max_cycles -= 50;
					cycle_length = 1000.0 / max_cycles;
				}
			}
			//TODO: Add chip8 keys
		}
	}
	u32 kUp = hidKeysUp();
	if (kUp != 0) {
		//TODO: Add chip8 keys
	}

	hidExit();
	gfxExit();
	return 0;
}
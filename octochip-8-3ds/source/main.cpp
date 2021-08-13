#include <stdio.h>
#include <string>
#include <3ds.h>
#include <citro2d.h>
#include <map>
#include "chip8.h"

//Top screen 50x30 characters
//Bottom screen 40x30 characters

//Declare 3ds variables
PrintConsole botScr;
C3D_RenderTarget* topScr;
const int MODIFIER = 6;

//Declare color variables
u32 BLACK = C2D_Color32(0, 0, 0, 255);
u32 WHITE = C2D_Color32(255, 255, 255, 255);
u32 GRAY = C2D_Color32(90, 90, 90, 255);

//Declare chip8 variables
chip8 myChip8; //The one and only
std::map<u32, int> keymap = { //The keymap
	{ KEY_UP, 0x1 },
	{ KEY_DOWN, 0x4 },
	{ KEY_X, 0xC },
	{ KEY_B, 0xD }
};

//Keyboard callback function
static SwkbdCallbackResult loadROMcallback(void* user, const char** ppMessage, const char* text, size_t textlen) {
	if (!myChip8.loadApplication(text)) {
		*ppMessage = "Unable to load file. Try again.";
		return SWKBD_CALLBACK_CONTINUE;
	}
	*ppMessage = "Loaded file.";
	return SWKBD_CALLBACK_OK;
}

//Main
int main(int argc, char* argv[]) {
	bool quit = false; //Quit flag

	//Initialize display
	gfxInitDefault();
	hidInit();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	consoleInit(GFX_BOTTOM, &botScr);
	topScr = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

	//Control list
	char control_list[] =
		"OctoChip-8\n\n"
		"A     : Run normally\n"
		"Y     : Run one cycle (Pause-ish)\n"
		"A+Y   : Bind keys\n"
		"L/R   : Change speed by 50\n"
		"Touch : Toggle Registers\n"
		"Select: Load ROM\n"
		"Start : Quit\n";
	printf("%s", control_list);

	
	

	//Main loop
	int mode = 1; //Regular vs Cycle by cycle, see Modes comment below
	u64 count_ticks = svcGetSystemTick(); //Used for counting how many cycles actually execute per second
	int cycles = 0; //Used for counting how many cycles actually execute per second
	bool display_registers = true; //Whether the registers should be displayed
	int max_cycles = 500; //Maximum cycles per second
	double cycle_length = 1000.0 / max_cycles; //Ticks per cycle
	u64 limit_ticks = svcGetSystemTick(); //Used for limiting how many cycles per second
	//Modes:
	//0 - Run normally
	//1 - Don't run cycle until Y is pressed
	//2 - Y has been pressed, run one cycle
	//3 - Unknown opcode, press start to quit
	while (aptMainLoop() && !quit) {
		hidScanInput();

		//Execute button presses
		u32 kDown = hidKeysDown();
		if (kDown != 0) {
			if (kDown & KEY_START) { //Quit Program
				quit = true;
			}
			if (kDown & KEY_SELECT) { //Load chip8 ROM                 DOESNT SHOW ERROR IF ROM FILE NOT FOUND
				mode = 1;
				static SwkbdState swkbd;
				static char filename[256] = "/chip8/";
				SwkbdButton button = SWKBD_BUTTON_NONE;

				swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, 255);
				swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
				swkbdSetFeatures(&swkbd, SWKBD_ALLOW_HOME | SWKBD_ALLOW_RESET | SWKBD_ALLOW_POWER | SWKBD_DEFAULT_QWERTY);
				swkbdSetFilterCallback(&swkbd, loadROMcallback, NULL);

				do {
					swkbdSetInitialText(&swkbd, filename);
					button = swkbdInputText(&swkbd, filename, sizeof(filename));

					if (button != SWKBD_BUTTON_NONE) {
						break;
					}

					SwkbdResult res = swkbdGetResult(&swkbd);
					if (res == SWKBD_RESETPRESSED) { //Reset combo pressed (L+R+Start)
						quit = true;
						aptSetChainloaderToSelf();
						break;
					} else if (res != SWKBD_HOMEPRESSED && res != SWKBD_POWERPRESSED) { //Reset combo or home/power was not pressed
						break;
					}
					quit = !aptMainLoop();
				} while (!quit);
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
					printf("\x1b[1;1H%s", control_list);
				} else {
					printf("\x1b[11;1H\x1b[0J");
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
			for (std::map<u32, int>::iterator i = keymap.begin(); i != keymap.end(); i++) { //Chip8 key was pressed
				if (kDown & i->first) {
					myChip8.key[i->second] = 1;
				}
			}
		}

		u32 kDR = hidKeysDownRepeat();
		if ((kDR & KEY_A) && (kDR & KEY_Y)) { //Bind keys
			printf("\x1b[11;1H\x1b[0J");
			printf("\x1b[16;12HPress key to bind");
			printf("\x1b[17;13Hto Chip-8 key 0");
			printf("\x1b[21;12HTouch to skip key");
			printf("\x1b[23;13HPress Y to end");
			keymap.clear();

			bool end = false;
			int key = 0x0;
			while (aptMainLoop() && !end && key <= 0xF) {
				hidScanInput();
				u32 kDown = hidKeysDown();
				if (kDown != 0) {
					if (kDown & KEY_START) { //Quit Program
						end = true;
						quit = true;
					} else if (kDown & KEY_Y) { //End key binding
						end = true;
					} else if (kDown & KEY_TOUCH) { //Skip key
						++key;
						printf("\x1b[17;27H%X", key);
					} else if ((kDown & (kDown - 1)) == 0){
						keymap[kDown] = key;
						++key;
						printf("\x1b[17;27H%X", key);
					}
				}
			}
			printf("\x1b[11;1H\x1b[0J");
			for (std::map<u32, int>::iterator i = keymap.begin(); i != keymap.end(); ++i) { //Chip8 key was pressed
				printf("%i: %X\n", (int)log2(i->first), i->second);
			}
		}

		u32 kUp = hidKeysUp();
		if (kUp != 0) { //Chip8 key was released
			for (std::map<u32, int>::iterator i = keymap.begin(); i != keymap.end(); i++) { //Chip8 key was pressed
				if (kUp & i->first) {
					myChip8.key[i->second] = 0;
				}
			}
		}


		if (mode == 0 || mode == 2) {
			//Emulate a cycle
			if (!myChip8.emulateCycle()) {
				mode = 3;
			}

			//Update chip8 display if it has changed
			if (myChip8.draw_flag) {
				C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
				C2D_TargetClear(topScr, GRAY);
				C2D_SceneBegin(topScr);

				C2D_DrawRectSolid(8, 24, 0, 64 * MODIFIER, 32 * MODIFIER, BLACK);

				for (int y = 0; y < 32; ++y) {
					for (int x = 0; x < 64; ++x) {
						if (myChip8.gfx[x + (y * 64)] == 1) {
							C2D_DrawRectSolid((x * MODIFIER) + 8, (y * MODIFIER) + 24, 0, MODIFIER, MODIFIER, WHITE);
						}
					}
				}

				C3D_FrameEnd(0);
				//Set draw flag to false
				myChip8.draw_flag = false;
			}

			if (display_registers) {
				//Display registers
				unsigned short values[40] = { 0 };
				myChip8.getRegisters(values);
				printf("\x1b[11;1HI : %04X   PC: %04X   OP: %04X\nDT: %04X   ST: %04X   SP: %04X", values[34], values[33], values[32], values[38], values[39], values[35]);
				for (int i = 0; i < 8; ++i) {
					printf("\x1b[%i;1HV%X: %02X   V%X: %02X   S%X: %04X   S%X: %04X", i + 14, i, values[i], i + 8, values[i + 8], i, values[i + 16], i + 8, values[i + 24]);
				}

				//Display cycles per second
				++cycles;
				if (svcGetSystemTick() - count_ticks > 1000) {
					printf("\x1b[23;1HSpeed: %3i    Cycles per second: %4i", max_cycles, cycles);
					cycles = 0;
					count_ticks = svcGetSystemTick();
				}
			}

			//Limit speed of emulation
			while (svcGetSystemTick() - limit_ticks < cycle_length) continue;
			limit_ticks = svcGetSystemTick();

			//Dun't run cycle until space is pressed
			if (mode == 2) {
				mode = 1;
			}

			//gspWaitForVBlank();
		}
	}

	C2D_Fini();
	C3D_Fini();
	hidExit();
	gfxExit();
	return 0;
}
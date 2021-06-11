#include <stdio.h>

int main(int argc, char** argv) {
	printf("Chip-8 Disassembler\n\n");

	FILE* rom_file;
	unsigned char memory[4096];
	#pragma warning(suppress : 4996)
	rom_file = fopen("C:\\Users\\switc\\Desktop\\Programming\\Projects\\octochip-8\\myChip8-bin-src\\invaders.c8", "rb");

	fread(&memory, 4096, 1, rom_file);
	printf(" 0000  ");
	for (int i = 0; i < 4096; ++i) {
		printf("%02x ", memory[i]);
		if ((i+1) % 0x10 == 0) {
			printf("\n %04x  ", i+1);
		}
	}
	


	fclose(rom_file);
	return 0;
}
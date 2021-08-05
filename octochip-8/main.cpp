#include <stdio.h>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <map>
#include "chip8.h"

//Texture wrapper class. This comes from Lazy Foo' Productions (http://lazyfoo.net/)
class LTexture {
public:
	LTexture(); //Initialize variables
	~LTexture(); //Deallocate memory

	bool loadFromFile(std::string path); //Loads image
	bool loadFromRenderedText(std::string textureText, SDL_Color textColor); //Creates image from font string
	void free(); //Deallocates texture
	void setColor(Uint8 red, Uint8 green, Uint8 blue); //Sets color modulation
	void setBlendMode(SDL_BlendMode blending); //Set blending
	void setAlpha(Uint8 alpha); //Set alpha modulation
	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE); //Renders texture at given point

	int getWidth();
	int getHeight();

private:
	SDL_Texture* mTexture;
	int mWidth;
	int mHeight;
};

//Declare graphics variables
const int SCREEN_WIDTH = 512; //formerly 1024
const int SCREEN_HEIGHT = 426; //formerly 512
const int SCREEN_HEIGHT_SMALL = 256;
const int MODIFIER = 8;
const unsigned char TRANS_COLORS[3] = { 54, 57, 63 }; //RGB of the color to treat as transparent when loading images
const char* FONT_PATH = "C:/Windows/Fonts/consola.ttf";
const int FONT_SIZE = 18;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font;
LTexture textTexture;

const SDL_Color BLACK = { 0, 0, 0 };
const SDL_Color WHITE = { 255, 255, 255 };
const SDL_Color RED = { 255, 0, 0 };
const SDL_Color GREEN = { 0, 255, 0 };
const SDL_Color BLUE = { 0, 0, 255 };

//Initialize Display
bool init_SDL() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	} else {
		//Create window
		window = SDL_CreateWindow("OctoChip-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL) {
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			return false;
		} else {
			//Create renderer for window
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (renderer == NULL) {
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				return false;
			} else {
				//Initialize renderer color
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags)) {
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
				}

				//Initialize SDL_ttf
				if (TTF_Init() == -1) {
					printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
				} else {
					font = TTF_OpenFont(FONT_PATH, FONT_SIZE);
					if (font == NULL) {
						printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
					}
				}
			}
		}
	}
	return true;
}

//Close SDL
void close_SDL() {
	TTF_CloseFont(font);
	font = NULL;
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
	SDL_DestroyWindow(window);
	window = NULL;

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

//Initialize LTexture
LTexture::LTexture() {
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

//Deallocates LTexture
LTexture::~LTexture() {
	free();
}

//Load textures from specified path
bool LTexture::loadFromFile(std::string path) {
	free();
	SDL_Texture* newTexture = NULL;
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());

	if (loadedSurface == NULL) {
		printf("Unable to load image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
	} else {
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, TRANS_COLORS[0], TRANS_COLORS[1], TRANS_COLORS[2]));

		newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (newTexture == NULL) {
			printf("Unable to create new texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		} else {
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}
		SDL_FreeSurface(loadedSurface);
	}

	mTexture = newTexture;
	return mTexture != NULL;
}

//Creates image from font string
bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor) {
	free();
	SDL_Surface* textSurface = TTF_RenderText_Solid(font, textureText.c_str(), textColor);
	if (textSurface == NULL) {
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	} else {
		mTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
		if (mTexture == NULL) {
			printf("Unable to create texture from rendered text! SDL Error %s\n", SDL_GetError());
		} else {
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}
		SDL_FreeSurface(textSurface);
	}
	return mTexture != NULL;
}

//Deallocates texture
void LTexture::free() {
	if (mTexture != NULL) {
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

//Sets color modulation
void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue) {
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

//Set blending
void LTexture::setBlendMode(SDL_BlendMode blending) {
	SDL_SetTextureBlendMode(mTexture, blending);
}

//Set alpha modulation
void LTexture::setAlpha(Uint8 alpha) {
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

//Render image
void LTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip) {
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	if (clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	SDL_RenderCopyEx(renderer, mTexture, clip, &renderQuad, angle, center, flip);
}

//Get width of texture
int LTexture::getWidth() {
	return mWidth;
}

//Get height of texture
int LTexture::getHeight() {
	return mHeight;
}

//Declare chip8 variables
chip8 myChip8; //The one and only
SDL_Rect chip8Rect = { 0, 0, 512, 256 }; //The chip8 display
SDL_Rect regRect = { 0, 256, 512, 256 }; //The register display
SDL_Rect memRect = { 512, 0, 512, 512 }; //The memory display
SDL_Rect chip8Border = { -1, -1, 514, 258 }; //The border around chip8Rect
std::map<int, int> keymap = { //The keymap
		{ SDLK_1, 0x1 },
		{ SDLK_2, 0x2 },
		{ SDLK_3, 0x3 },
		{ SDLK_4, 0xC },
		{ SDLK_q, 0x4 },
		{ SDLK_w, 0x5 },
		{ SDLK_e, 0x6 },
		{ SDLK_r, 0xD },
		{ SDLK_a, 0x7 },
		{ SDLK_s, 0x8 },
		{ SDLK_d, 0x9 },
		{ SDLK_f, 0xE },
		{ SDLK_z, 0xA },
		{ SDLK_x, 0x0 },
		{ SDLK_c, 0xB },
		{ SDLK_v, 0xF }
};


//Main
int main(int argc, char** argv) {
	printf("OctoChip-8\n\n");

	//Check if enough arguments are supplied
	if (argc < 2) {
		printf("Usage: OctoChip-8.exe <ROM path>\n");
		return 1;
	}

	//Initialize display
	if (!init_SDL()) {
		printf("Failed to initialize SDL!\n");
		return 1;
	}

	//Load chip8 ROM
	if (!myChip8.loadApplication(argv[1])) {
		return 1;
	}

	//Instructions screen
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);
	textTexture.loadFromRenderedText("Enter:   Run normally", BLACK);
	textTexture.render(18, 18);
	textTexture.loadFromRenderedText("Space:   Run one cycle", BLACK);
	textTexture.render(18, 38);
	textTexture.loadFromRenderedText("Control: Toggle registers", BLACK);
	textTexture.render(18, 58);
	textTexture.loadFromRenderedText("+/-: Change speed by 50", BLACK);
	textTexture.render(18, 78);
	textTexture.loadFromRenderedText("Chip-8:        Keyboard:", BLACK);
	textTexture.render(18, 118);
	textTexture.loadFromRenderedText("+-+-+-+-+      +-+-+-+-+", BLACK);
	textTexture.render(18, 138);
	textTexture.loadFromRenderedText("|1|2|3|C|      |1|2|3|4|", BLACK);
	textTexture.render(18, 158);
	textTexture.loadFromRenderedText("+-+-+-+-+      +-+-+-+-+", BLACK);
	textTexture.render(18, 178);
	textTexture.loadFromRenderedText("|4|5|6|D|      |Q|W|E|R|", BLACK);
	textTexture.render(18, 198);
	textTexture.loadFromRenderedText("+-+-+-+-+  =>  +-+-+-+-+", BLACK);
	textTexture.render(18, 218);
	textTexture.loadFromRenderedText("|7|8|9|E|      |A|S|D|F|", BLACK);
	textTexture.render(18, 238);
	textTexture.loadFromRenderedText("+-+-+-+-+      +-+-+-+-+", BLACK);
	textTexture.render(18, 258);
	textTexture.loadFromRenderedText("|A|0|B|F|      |Z|X|C|V|", BLACK);
	textTexture.render(18, 278);
	textTexture.loadFromRenderedText("+-+-+-+-+      +-+-+-+-+", BLACK);
	textTexture.render(18, 298);
	SDL_RenderPresent(renderer);


	//Stores the strings to display the register values, if displayed in hex instead of decimal then 50 is larger than needed
	char regRow[9][50];
	//Names of the registers in the third column
	char regCol3[8][3] = { "OP", "PC", "I", "SP", "", "", "DT", "ST" };


	//Main loop
	bool quit = false; //Quit flag
	SDL_Event e; //SDL event
	int mode = 1; //Regular vs Cycle by cycle, see Modes comment below
	int regColor = 175; //Background color of the registers
	Uint32 count_ticks = SDL_GetTicks(); //Used for counting how many cycles actually execute per second
	int cycles = 0; //Used for counting how many cycles actually execute per second
	bool display_registers = true; //Whether the registers should be displayed
	int max_cycles = 500; //Maximum cycles per second
	double cycle_length = 1000.0 / max_cycles; //Ticks per cycle
	Uint32 limit_ticks = SDL_GetTicks(); //Used for limiting how many cycles per second
	//Modes:
	//0 - Run normally
	//1 - Don't run cycle until space is pressed
	//2 - Space has been pressed, run one cycle
	//3 - Unknown opcode, press enter to quit
	while (!quit) {
		//Event loop
		while (SDL_PollEvent(&e) != 0) {
			switch (e.type) {
			case SDL_QUIT:
				quit = true; break;

			case SDL_KEYDOWN:
				switch (e.key.keysym.sym) {
				case SDLK_ESCAPE: //Quit program
					quit = true; break;

				case SDLK_RETURN: //Run normally, or quit if mode is 3
					if (mode == 3) {
						quit = true;
					} else {
						mode = 0;
					} break;

				case SDLK_SPACE: //Space has been pressed, run one cycle
					mode = 2; break;

				case SDLK_LCTRL: //Toggle register display
				case SDLK_RCTRL:
					display_registers = !display_registers;
					if (display_registers) {
						SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
					} else {
						SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT_SMALL);
					} break;

				case SDLK_EQUALS: //Increase speed by 50
					max_cycles += 50;
					cycle_length = 1000.0 / max_cycles;
					break;

				case SDLK_MINUS: //Decrease speed by 50
					if (max_cycles > 50) {
						max_cycles -= 50;
						cycle_length = 1000.0 / max_cycles;
					} break;

				default: //Chip8 key was pressed
					if (keymap.count(e.key.keysym.sym) == 1) {
						myChip8.key[keymap[e.key.keysym.sym]] = 1;
					} break;
				} break;

			case SDL_KEYUP: //Chip8 key was released
				if (keymap.count(e.key.keysym.sym) == 1) {
					myChip8.key[keymap[e.key.keysym.sym]] = 0;
				} break;
			}
		}

		if (mode == 0 || mode == 2) {
			//Emulate a cycle
			if (!myChip8.emulateCycle()) {
				mode = 3;
				regColor = 150;
			}

			//Update chip8 display if it has changed
			if (myChip8.draw_flag) {
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderFillRect(renderer, &chip8Rect);
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				for (int y = 0; y < 32; ++y) {
					for (int x = 0; x < 64; ++x) {
						if (myChip8.gfx[x + (y * 64)] == 1) {
							SDL_Rect pixelRect = { x * MODIFIER, y * MODIFIER, MODIFIER, MODIFIER };
							SDL_RenderFillRect(renderer, &pixelRect);
						}
					}
				}
			}

			if (display_registers) {
				//Display registers
				SDL_SetRenderDrawColor(renderer, 175, regColor, regColor, 255);
				SDL_RenderFillRect(renderer, &regRect);
				unsigned short values[40] = { 0 };
				myChip8.getRegisters(values);
				for (int i = 0; i < 8; ++i) {
					snprintf(regRow[i], 50, "V%X: %02X   V%X: %02X   S%X: %04X   S%X: %04X   %2s: %04X", i, values[i], i + 8, values[i + 8], i, values[i + 16], i + 8, values[i + 24], regCol3[i], values[i + 32]);
					textTexture.loadFromRenderedText(regRow[i], BLACK);
					textTexture.render(4, 260 + (18 * i));
				}

				//Display cycles per second
				++cycles;
				if (SDL_GetTicks() - count_ticks > 1000) {
					snprintf(regRow[8], 50, "Speed: %3i               Cycles per second: %4i", max_cycles, cycles);
					cycles = 0;
					count_ticks = SDL_GetTicks();
				}
				textTexture.loadFromRenderedText(regRow[8], BLACK);
				textTexture.render(4, 404);
			}

			//Update screen
			if (display_registers || myChip8.draw_flag) {
				SDL_RenderPresent(renderer);

				//Set draw flag to false
				myChip8.draw_flag = false;
			}

			//Limit speed of emulation
			while (SDL_GetTicks() - limit_ticks < cycle_length) continue;
			limit_ticks = SDL_GetTicks();

			//Don't run cycle until space is pressed
			if (mode == 2) {
				mode = 1;
			}
		}
	}
	
	printf("\n\nGoodbye.\n");
	close_SDL();
	return 0;
}
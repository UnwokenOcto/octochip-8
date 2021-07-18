#include <stdio.h>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
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
const int SCREEN_HEIGHT = 406; //formerly 512
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

	if (!myChip8.loadApplication(argv[1])) {
		return 1;
	}

	//Stores the strings to display the register values, if displayed in hex instead of decimal then 50 is larger than needed
	char regRow[8][50];
	//Names of the registers in the third column
	char regCol3[8][3] = { "OP", "PC", "I", "SP", "", "", "DT", "ST" };


	//Main loop
	bool quit = false;
	SDL_Event e;
	int mode = 1;
	int regColor = 175;
	//Modes:
	//0 - Run normally
	//1 - Don't run cycle until space is pressed
	//2 - Space has been pressed, run one cycle
	//3 - Unknown opcode, press enter to quit
	while (!quit) {
		//Event loop
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
			} else if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_ESCAPE) { //Quit program
					quit = true;
				} else if (e.key.keysym.sym == SDLK_SPACE) { //Space has been pressed, run one cycle
					mode = 2;
				} else if (e.key.keysym.sym == SDLK_RETURN) { //Run normally, or quit if mode is 3
					if (mode == 3) {
						quit = true;
					} else {
						mode = 0;
					}
				}
			}
		}

		if (mode == 0 || mode == 2) {
			//Emulate a cycle
			if (!myChip8.emulateCycle()) {
				mode = 3;
				regColor = 150;
			}

			//Clear register and memory displays
			SDL_SetRenderDrawColor(renderer, 175, regColor, regColor, 255);
			SDL_RenderFillRect(renderer, &regRect);
			//SDL_RenderFillRect(renderer, &memRect);

			//SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
			//SDL_RenderDrawRect(renderer, &chip8Border);


			//Display registers
			unsigned short values[40] = { 0 };
			myChip8.getRegisters(values);
			for (int i = 0; i < 8; ++i) {
				snprintf(regRow[i], 50, "V%X: %02X   V%X: %02X   S%X: %04X   S%X: %04X   %2s: %04X", i, values[i], i + 8, values[i + 8], i, values[i + 16], i + 8, values[i + 24], regCol3[i], values[i + 32]);
				textTexture.loadFromRenderedText(regRow[i], BLACK);
				textTexture.render(4, 260 + (18 * i));
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

				//Set draw flag to false
				myChip8.draw_flag = false;
			}

			//Update screen
			SDL_RenderPresent(renderer);

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
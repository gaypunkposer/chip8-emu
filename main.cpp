#include <iostream>
#include <fstream>
#include "chip8.h"
#include <SDL.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 320;

chip8 emu;
SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
bool quit = false;
SDL_Keycode inputMap[] = {SDLK_1, SDLK_2, SDLK_3, SDLK_4,
                          SDLK_q, SDLK_w, SDLK_e, SDLK_r,
                          SDLK_a, SDLK_s, SDLK_d, SDLK_f,
                          SDLK_z, SDLK_x, SDLK_c, SDLK_v};

void initializeGraphics();

void loadGame(char *string);

void draw();

void getInput();

int main(int argc, char *args[]) {
    initializeGraphics();
    emu.initialize();

    loadGame(args[1]);

    while (!quit) {
        while (emu.isRunning) {
            getInput();

            emu.emulateCycle();

            if (emu.drawFlag) {
                draw();
            }

            SDL_Delay(1);
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void getInput() {
    unsigned char input[16];
    for (int i = 0; i < 16; i++) {
        input[i] = 0;
    }

    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            emu.isRunning = false;
            quit = true;
        } else if (e.type == SDL_KEYDOWN) {
            printf("Key pressed: %d\n", e.key.keysym.sym);
            for (int i = 0; i < 16; i++) {
                if (inputMap[i] == e.key.keysym.sym) {
                    input[i] = 1;
                }
            }
        }
    }

    for (int i = 0; i < 16; i++) {
        if (input[i] == 1) {
            emu.s.key[i] = 1;
        }
    }
}


void draw() {
    //clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    //draw rectangles
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            unsigned char pix = emu.s.gfx[64 * i + j];
            if (pix > 0) {
                SDL_Rect rect;
                rect.x = j * 10;
                rect.y = i * 10;
                rect.w = 10;
                rect.h = 10;
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

void loadGame(char *file) {
    std::ifstream in;
    in.open(file, std::ios::in | std::ios::binary);
    if (in.is_open()) {
        in.seekg(0, in.end);
        int size = in.tellg();
        in.seekg(0, in.beg);

        char *buff = (char *) malloc(sizeof(char) * size);
        in.read(buff, size);
        in.close();
        emu.loadGame((unsigned char *) buff, size);
    } else {
        perror("Failed to open file.\n");
        exit(1);
    }
}

void initializeGraphics() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize.\n");
        exit(1);
    }

    window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        printf("SDL could not create window.\n");
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
}

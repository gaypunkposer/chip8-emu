//
// Created by Jessica on 5/26/2020.
//

#include "chip8.h"

void chip8::initialize() {
    s.pc = 512;
    s.opcode = 0;
    s.I = 0;
    s.sp = 0;

    //clear memory
    for (int i = 0; i < 4096; i++) {
        s.memory[i] = 0;
    }

    clearScreen();

    //clear V registers and stack
    for (int i = 0; i < 16; i++) {
        s.V[i] = 0;
        s.stack[i] = 0;
    }

    //clear input
    for (int i = 0; i < 16; i++) {
        s.key[i] = 0;
    }

    //load the font set
    for (int i = 0; i < 80; i++) {
        s.memory[i] = fontset[i];
    }

    //seed randomness
    srand(time(NULL));
    isRunning = true;
    drawFlag = true;
}

void chip8::loadGame(const unsigned char *data, int size) {
    for (int i = 0; i < size; i++) {
        //chip8 reserves the first 512 bytes for the interpreter
        s.memory[i + 0x200] = data[i];
    }
}

void chip8::emulateCycle() {
    //op code is 2 bytes, load both bytes from memory
    s.opcode = s.memory[s.pc] << 8 | s.memory[s.pc + 1];
    drawFlag = false;

    switch ((s.opcode & 0xF000) >> 12) {
        case 0x0: {
            if (s.opcode == 0x00E0) {
                drawFlag = true;
                clearScreen();
                s.pc += 2;
            } else if (s.opcode == 0x00EE) {
                //return from function call
                s.sp--;
                s.pc = s.stack[s.sp] + 2;
                //s.stack[s.sp] = 0;
            } else {
                printf("Not implemented.\n");
            }
            break;
        }
        case 0x1: {
            //jump to address
            s.pc = s.opcode & 0x0FFF;
            break;
        }
        case 0x2: {
            //call function
            s.stack[s.sp] = s.pc;
            s.sp++;
            s.pc = s.opcode & 0x0FFF;
            break;
        }
        case 0x3: {
            //skip next instruction if VX == val
            unsigned char val = s.opcode & 0x00FF;
            unsigned char v = (s.opcode & 0x0F00) >> 8;
            if (s.V[v] == val) {
                s.pc += 4; // skip the next instruction
            } else {
                s.pc += 2;
            }
            break;
        }
        case 0x4: {
            //skip next instruction if VX != val
            unsigned char val = s.opcode & 0x00FF;
            unsigned char v = (s.opcode & 0x0F00) >> 8;
            if (s.V[v] != val) s.pc += 4; // skip the next instruction
            else s.pc += 2;
            break;
        }
        case 0x5: {
            //skip next instruction if VX == VY
            unsigned char v1 = (s.opcode & 0x0F00) >> 8;
            unsigned char v2 = (s.opcode & 0x00F0) >> 4;
            if (s.V[v1] == s.V[v2]) s.pc += 4;
            else s.pc += 2;
            break;
        }
        case 0x6: {
            //set VX to val
            unsigned char v = (s.opcode & 0x0F00) >> 8;
            unsigned char val = s.opcode & 0x00FF;
            s.V[v] = val;
            s.pc += 2;
            break;
        }
        case 0x7: {
            //add val to VX
            unsigned char v = (s.opcode & 0x0F00) >> 8;
            unsigned char val = s.opcode & 0x00FF;
            s.V[v] += val;
            s.pc += 2;
            break;
        }
        case 0x8: {
            unsigned char v1 = (s.opcode & 0x0F00) >> 8;
            unsigned char v2 = (s.opcode & 0x00F0) >> 4;
            switch (s.opcode & 0x000F) {
                case 0x0:
                    //set VX to VY
                    s.V[v1] = s.V[v2];
                    break;
                case 0x1:
                    //set VX to VX OR VY
                    s.V[v1] = s.V[v1] | s.V[v2];
                    break;
                case 0x2:
                    //set VX to VX AND VY
                    s.V[v1] = s.V[v1] & s.V[v2];
                    break;
                case 0x3:
                    //set VX to VX XOR VY
                    s.V[v1] = s.V[v1] ^ s.V[v2];
                    break;
                case 0x4:
                    //add VY to VX
                    if (s.V[v2] > (0xFF - s.V[v1]))
                        s.V[0xF] = 1; //if v1+v2 would result in a number greater than 255, set carry bit
                    else s.V[0xF] = 0;
                    s.V[v1] += s.V[v2];
                    break;
                case 0x5:
                    //subtract VY from VX
                    if (s.V[v2] > s.V[v1]) s.V[0xF] = 0; //if v1-v2 would result in a number less than 0, set carry bit
                    else s.V[0xF] = 1;
                    s.V[v1] -= s.V[v2];
                    break;
                case 0x6:
                    //set VF to VX's LSB, right shift VX
                    s.V[0xF] = (s.V[v1] & 0x0001);
                    s.V[v1] = s.V[v1] >> 1;
                    break;
                case 0x7:
                    //set VX to VY - VX
                    if (s.V[v1] > s.V[v2]) s.V[0xF] = 0; //if v2-v1 would result in a number less than 0, set carry bit
                    else s.V[0xF] = 1;
                    s.V[v1] = s.V[v2] - s.V[v1];
                    break;
                case 0xE:
                    //set VF to VX's MSB, left shift VX
                    s.V[0xF] = s.V[v1] >> 7;
                    s.V[v1] = s.V[v1] << 1;
                    break;
                default:
                    printf("Unknown instruction 0x%X", s.opcode);
            }
            s.pc += 2;
            break;
        }
        case 0x9: {
            //skip next instruction if VX != VY
            unsigned char v1 = (s.opcode & 0x0F00) >> 8;
            unsigned char v2 = (s.opcode & 0x00F0) >> 4;
            if (s.V[v1] != s.V[v2]) s.pc += 4;
            else s.pc += 2;
            break;
        }
        case 0xA: {
            //set I register to address NNN
            s.I = (s.opcode & 0x0FFF);
            s.pc += 2;
            break;
        }
        case 0xB: {
            //set PC to V0 + NNN
            s.pc = s.V[0] + (s.opcode & 0x0FFF);
            break;
        }
        case 0xC: {
            //set VX to random number & NN
            unsigned char v = (s.opcode & 0x0F00) >> 8;
            unsigned char r = rand();
            s.V[v] = r & (s.opcode & 0x00FF);
            s.pc += 2;
            break;
        }
        case 0xD: {
            //draw sprite to screen with X, Y coordinate and height
            unsigned short x = s.V[(s.opcode & 0x0F00) >> 8];
            unsigned short y = s.V[(s.opcode & 0x00F0) >> 4];
            unsigned short h = s.opcode & 0x000F;
            drawSprite(x, y, h);
            drawFlag = true;
            s.pc += 2;
            break;
        }
        case 0xE: {
            //input
            unsigned char v = (s.opcode & 0x0F00) >> 8;
            //("checking for input\n");
            if ((s.opcode & 0x00FF) == 0x9E) {
                //if key at VX has been pressed, skip next instruction
                if (s.key[s.V[v]] != 0) {
                    s.pc += 4;
                } else {
                    s.pc += 2;
                }
                s.key[s.V[v]] = 0;
            } else if ((s.opcode & 0x00FF) == 0xA1) {
                //if key at VX hasn't been pressed, skip next instruction
                if (s.key[s.V[v]] == 0) {
                    s.pc += 4;
                } else {
                    s.pc += 2;
                }
                s.key[s.V[v]] = 0;
            } else {
                printf("Unknown instruction 0x%X", s.opcode);
            }
            break;
        }
        case 0xF: {
            unsigned char v = (s.opcode & 0x0F00) >> 8;
            switch (s.opcode & 0x00FF) {
                case 0x07: {
                    //set VX to delay_timer
                    s.V[v] = s.delay_timer;
                    s.pc += 2;
                    break;
                }
                case 0x0A: {
                    //block execution until key pressed
                    int key = getBlockingKey();
                    if (key != -1) {
                        s.V[v] = key;
                        s.pc += 2;
                    }
                    break;
                }
                case 0x15: {
                    //set delay_timer to VX
                    s.delay_timer = s.V[v];
                    s.pc += 2;
                    break;
                }
                case 0x18: {
                    //set sound_timer to VX
                    s.sound_timer = s.V[v];
                    s.pc += 2;
                    break;
                }
                case 0x1E: {
                    //add VX to I
                    if (s.I + s.V[v] > 0xFFF) s.V[0xF] = 1; //set VF if S.I + VX > 0xFFF
                    else s.V[0xF] = 0;
                    s.I += s.V[v];
                    s.pc += 2;
                    break;
                }
                case 0x29:
                    //get sprite at addr VX
                    s.I = s.V[v] * 5;
                    s.pc += 2;
                    break;
                case 0x33:
                    //stores binary-coded decimal representation of VX
                    //this was copied, should go back and understand what it does.
                    s.memory[s.I] = s.V[v] / 100;
                    s.memory[s.I + 1] = (s.V[v] / 10) % 10;
                    s.memory[s.I + 2] = (s.V[v] % 100) % 10;
                    s.pc += 2;
                    break;
                case 0x55:
                    //dump V0->VX (inclusive) to memory
                    for (int i = 0; i <= v; i++) {
                        s.memory[s.I + i] = s.V[i];
                    }
                    s.pc += 2;
                    break;
                case 0x65:
                    //load into V0->VX (inclusive) from memory
                    for (int i = 0; i <= v; i++) {
                        s.V[i] = s.memory[s.I + i];
                    }
                    s.pc += 2;
                    break;
            }
            break;
        }
    }
    if (s.delay_timer > 0) s.delay_timer--;
    if (s.sound_timer > 0) s.sound_timer--;
}

void chip8::clearScreen() {
    for (int i = 0; i < 2048; i++) {
        s.gfx[i] = 0;
    }
    printf("Clearing screen\n");
}

void chip8::drawSprite(unsigned short x, unsigned short y, unsigned short h) {
    //draw each row of the sprite
    s.V[0xF] = 0;
    for (int i = 0; i < h; i++) {
        unsigned char row = s.memory[s.I + i];
        int start = ((y + i) * 64) + x;
        for (int j = 0; j < 8; j++) {
            unsigned short pix = row & (0x80 >> j);
            unsigned short end = pix ^ s.gfx[start + j];
            if (end == 0 && pix == 1) s.V[0xF] = 1;
            s.gfx[start + j] = end;
        }
    }
}

int chip8::getBlockingKey() {
    int key = -1;
    for (int i = 0; i < 16; i++) {
        if (s.key[i] != 0) {
            key = i;
            break;
        }
    }
    return key;
}

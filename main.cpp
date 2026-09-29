#include <iostream>
#include <stdint.h>
#include <vector>
#include <deque>
#include <fstream>
#include <cstring>
#include <atomic>
#include <thread>


#include <raylib.h>

#include "opcodes.h"
#include "font.h"
#include "tinyfiledialogs.h"

using namespace std;

atomic<bool> running{false};
atomic<int> stepReq{0};
atomic<uint64_t> targetCps{1000};
atomic<bool> shouldExit{false};

void drawCharVram(uint8_t* memory, char ch, int startX, int startY, uint8_t color) {
    if (static_cast<unsigned char>(ch) >= 128) return;

    for (int row = 0; row < 8; row++) {
        uint8_t line = font8x8_basic[static_cast<uint8_t>(ch)][row];
        for (int col = 0; col < 8; col++) {
            if (line & (1 << col)) {
                int px = startX + col;
                int py = startY + row;

                if (px >= 0 && px < 96 && py >= 0 && py < 96) {
                    uint16_t pixel_index = (py * 96) + px;
                    uint16_t byte_addr = 0xF000 + (pixel_index / 2);

                    uint8_t current_byte = memory[byte_addr];

                    if (pixel_index % 2 == 0) memory[byte_addr] = (current_byte & 0x0F) | ((color & 0x0F) << 4);
                    else memory[byte_addr] = (current_byte & 0xF0) | (color & 0x0F);
                }
            }
        }
    }
}

const Color PALETTE_4BIT[16] = {
    {0,0,0,255}, // 0x00
    {0,0,170,255}, // 0x01
    {0,170,0,255}, // 0x02
    {0,170,170,255}, // 0x03
    {170,0,0,255}, // 0x04
    {170,0,170,255}, // 0x05
    {170,85,0,255}, // 0x06
    {170,170,170,255}, // 0x07
    {85,85,85,255}, // 0x08
    {85,85,255,255}, // 0x09
    {85,255,85,255}, // 0x0A
    {85,255,255,255}, // 0x0B
    {255,85,85,255}, // 0x0C
    {255,85,255,255}, // 0x0D
    {255,255,85,255}, // 0x0E
    {255,255,255,255} // 0x0F
};

const int memSize = (1024 * 64) - 1;

uint16_t TBTW(uint8_t highByte, uint8_t lowByte) { return (static_cast<uint16_t>(highByte) << 8) | lowByte; }

struct CPU {
    uint16_t A;
    uint16_t X;
    uint16_t Y;

    uint16_t PC;
    deque<uint16_t> stack;

    bool FlagZ;
    bool FlagL;
    bool FlagM;

    uint8_t memory[memSize];

    void reset() {
        PC = 0x1000;
        A = X = Y = 0;
        FlagZ = FlagL = FlagM = false;
        memset(memory, 0, sizeof(memory));
    }

    void debug() {
        printf("/----------------------------------------------------\\\n");
        printf("|A 0x%04X | X 0x%04X | Y 0x%04X | PC 0x%04X | OP 0x%02X|\n", A, X, Y, PC, memory[PC]);
        printf("\\----------------------------------------------------/\n\n");
    }

    void step() {
        //debug();
        uint8_t opcode = memory[PC++];

        if(opcode == STA) { // Load value into A
            A = TBTW(memory[PC], memory[PC+1]);
            PC += 2; // 2 params
        } else if(opcode == STX) { // Load value into X
            X = TBTW(memory[PC], memory[PC+1]);
            PC += 2; // 2 params
        } else if(opcode == STY) { // Load value into Y
            Y = TBTW(memory[PC], memory[PC+1]);
            PC += 2; // 2 params
        } else if(opcode == NOP) { } // Do nothing
        else if(opcode == HLT) { exit(A); } // Stop processor

        else if(opcode == MAX) { X = A; } // Move A to X
        else if(opcode == MAY) { Y = A; } // Move A to Y
        else if(opcode == MXA) { A = X; } // Move X to A
        else if(opcode == MXY) { Y = X; } // Move X to Y
        else if(opcode == MYA) { A = Y; } // Move Y to A
        else if(opcode == MYX) { X = Y; } // Move Y to X

        else if(opcode == STM) { memory[X] = Y & 0xFF; memory[X+1] = (Y >> 8) & 0xFF; }
        else if(opcode == LDM) { Y = TBTW(memory[X], memory[X+1]); }

        else if(opcode == ADD) { A = A + Y; }
        else if(opcode == SUB) { A = A - Y; }
        else if(opcode == MUL) { A = A * Y; }
        else if(opcode == DIV) { X = A % Y; A = A / Y; }

        else if(opcode == SHL) { A = A << Y; }
        else if(opcode == SHR) { A = A >> Y; }

        else if(opcode == AND) { A = A & Y; }
        else if(opcode == OR) { A = A | Y; }
        else if(opcode == XOR) { A = A ^ Y; }
        else if(opcode == NOR) { A = ~(A | Y); }

        else if(opcode == JMP) { PC = TBTW(memory[PC], memory[PC+1]); }
        else if(opcode == CALL) { stack.push_front(PC + 2); PC = TBTW(memory[PC], memory[PC+1]); }
        else if(opcode == RET) { PC = stack.front(); stack.pop_front(); }

        else if(opcode == CMP) {
            if(A > Y) FlagM = true;
            if(A < Y) FlagL = true;
            if(A == Y) FlagZ = true;
        }
        else if(opcode == CLF) { FlagL = FlagM = FlagZ = false; }
        else if(opcode == JZ) { if(FlagZ) { PC = TBTW(memory[PC], memory[PC+1]); } else { PC += 2; } }
        else if(opcode == JNZ) { if(!FlagZ) { PC = TBTW(memory[PC], memory[PC+1]); } else { PC += 2; } }
        else if(opcode == JL) { if(FlagL) { PC = TBTW(memory[PC], memory[PC+1]); } else { PC += 2; } }
        else if(opcode == JM) { if(FlagM) { PC = TBTW(memory[PC], memory[PC+1]); } else { PC += 2; } }

        else if(opcode == PUSH) { stack.push_front(Y); }
        else if(opcode == POP) { Y = stack.front(); stack.pop_front(); }

        else if(opcode == INA) { A++; }
        else if(opcode == INX) { X++; }
        else if(opcode == INY) { Y++; }
        else if(opcode == DEA) { A--; }
        else if(opcode == DEX) { X--; }
        else if(opcode == DEY) { Y--; }

        else if(opcode == REPM) { // A is value, X is offset, Y is amt
            for(int i = 0; i < Y; i+=1) {
                memory[X + i] = A;
            }
        }

        else if(opcode == REPW) { // A is value, X is offset, Y is amt
            for(int i = 0; i < Y; i+=2) {
                memory[X + i] = (uint8_t)((A>>8) & 0xFF);
                memory[(X + i) + 1] = (uint8_t)(A & 0xFF);
            }
        }
        else if(opcode == CRD) { A = X = Y = 0; }
        else if(opcode == SYS) {
            switch(A) {
                case 1: { // Return random value in Y
                    Y = GetRandomValue(0x0000, 0xFFFF);
                    break;
                }
                case 2: { // Clear VRAM
                    memset(&memory[0xF000], Y, 0x0C00);
                    break;
                }
                case 3: { // Draw pixel, X, Y is pos, stack is color
                    uint16_t color = stack.front();
                    stack.pop_front();
                    if (X < 96 && Y < 96) {
                        uint16_t vram_offset = (Y * 96) + X;
                        memory[VRAM_BEGIN + vram_offset] = color;
                    }
                    break;
                }
                case 4: { // Get key pressed
                    A = GetKeyPressed();
                    break;
                }
                case 5: { // Check if key down (key is raylib code) X is key, Y is result
                    Y = IsKeyDown(X);
                    break;
                }
                case 6: { // Print char, stack top is char, X Y is pos
                    char ch = static_cast<char>(stack.front());
                    uint8_t color = 0x0F;
                    drawCharVram(memory, ch, X*8, Y*8, color);
                    break;
                }
                case 7: { // Print string, pointer pushed, X Y is pos
                    uint16_t textPtr = stack.front();
                    stack.pop_front();
                    int charX = X; int charY = Y;
                    uint8_t color = 0x0F;
                    while(true) {
                        char ch = static_cast<char>(memory[textPtr]);
                        if(ch == 0x00) break;
                        else if(ch == '\n') { charX = 0; charY++; }
                        else {
                            drawCharVram(memory, ch, charX*8, charY*8, color);
                            charX++;
                        }
                        textPtr++;
                    }
                    break;
                }
                case 8: { // Ms in Y
                    this_thread::sleep_for(chrono::milliseconds(Y));
                    break;
                }
            }
        }
    }
};

void cpuThreadLoop(CPU &cpu) {
    while (!shouldExit) {
        if (running) {
            uint64_t cps = targetCps.load();

            if (cps > 0) {
                auto ns_per_cycle = chrono::nanoseconds(1000000000ULL / cps);

                auto start_time = chrono::high_resolution_clock::now();

                cpu.step();

                auto elapsed = chrono::high_resolution_clock::now() - start_time;

                if (elapsed < ns_per_cycle) this_thread::sleep_for(ns_per_cycle - elapsed);
            } else cpu.step();

        } else if (stepReq > 0) {
            cpu.step();
            stepReq--;
        } else this_thread::sleep_for(chrono::milliseconds(1));
    }
}

constexpr int SCALE = 8;

void drawVRAM(const uint8_t* memory) {
    int vram_index = VRAM_BEGIN;

    for (int y = 0; y < 96; y++) {
        for (int x = 0; x < 96; x += 2) {
            uint8_t byteVal = memory[vram_index++];

            uint8_t colorIdx1 = (byteVal >> 4) & 0x0F;
            uint8_t colorIdx2 = byteVal & 0x0F;

            DrawRectangle(x * SCALE, y * SCALE, SCALE, SCALE, PALETTE_4BIT[colorIdx1]);
            DrawRectangle((x + 1) * SCALE, y * SCALE, SCALE, SCALE, PALETTE_4BIT[colorIdx2]);
        }
    }
}

bool loadProgram(CPU& cpu, const char* filePath) {
    ifstream file(filePath, ios::binary | ios::ate);

    if(!file.is_open()) {
        tinyfd_messageBox("Loading failed :<", "Couldnt open file :(", "ok", "error", 1);
        return false;
    }
    streamsize size = file.tellg();

    if(0x1000 + size > memSize) {
        tinyfd_messageBox("Loading failed :<", "File too large to fit :(", "ok", "error", 1);
        return false;
    }

    file.seekg(0, ios::beg);
    
    if(file.read(reinterpret_cast<char*>(&cpu.memory[0x1000]), size)) {
        tinyfd_messageBox("Loaded succesfully :>", "File has been successfully loaded :)", "ok", "info", 1);
        return true;
    } else {
        tinyfd_messageBox("Loading failed :<", "File failed to load :(", "ok", "error", 1);
        return false;
    }
}

int main() {

    init_opcodes();
    CPU cpu;

    cpu.reset();

    InitWindow(1024, 768, "vCPU");
    SetTargetFPS(60);
    const char* cps = tinyfd_inputBox("Enter CPS", "Enter CPU Hz", "1000");
    if(cps != NULL) {
        uint64_t cpsNum = (int)strtol(cps, NULL, 0);
        targetCps = cpsNum;
    } else targetCps = 1000;

    const char* filename = tinyfd_openFileDialog("Load program :>", ".", 0, {}, "Any Files (*.*)", 0);
    if(filename != NULL) {
        if(!loadProgram(cpu, filename)) return 1;
    }

    thread cpuThread(cpuThreadLoop, ref(cpu));

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground((Color){43, 43, 43, 255});
        drawVRAM(cpu.memory);

        DrawRectangle(778, 20, 236, 728, BLACK);
        DrawText(TextFormat("A: 0x%04X", cpu.A), 790, 30, 20, RAYWHITE);
        DrawText(TextFormat("X: 0x%04X", cpu.X), 790, 60, 20, RAYWHITE);
        DrawText(TextFormat("Y: 0x%04X", cpu.Y), 790, 90, 20, RAYWHITE);
        DrawText(TextFormat("PC: 0x%04X", cpu.PC), 790, 120, 20, RAYWHITE);
        DrawText(TextFormat("OP: 0x%04X", cpu.memory[cpu.PC]), 790, 150, 20, RAYWHITE);

        DrawText(TextFormat("Zero: %d", cpu.FlagZ), 790, 200, 20, RAYWHITE);
        DrawText(TextFormat("Less: %d", cpu.FlagL), 790, 230, 20, RAYWHITE);
        DrawText(TextFormat("More: %d", cpu.FlagM), 790, 260, 20, RAYWHITE);

        DrawText(TextFormat("Stack: 0x%04X", cpu.stack.front()), 790, 300, 20, RAYWHITE);

        DrawText(TextFormat("Next opcode: %s", opcodes[cpu.memory[cpu.PC]].c_str()), 790, 400, 20, RAYWHITE);
        DrawText(TextFormat("Next 2 Bytes: 0x%02X 0x%02X", cpu.memory[cpu.PC+1], cpu.memory[cpu.PC+2]), 790, 430, 18, RAYWHITE);

        if(!running) DrawText("Paused! Press P to unpause.", 790, 725, 15, RAYWHITE);
        EndDrawing();

        if(IsKeyPressed(KEY_S)) {
            running = false;
            stepReq++;
        }

        if(IsKeyPressed(KEY_Q)) {
            const char* input = tinyfd_inputBox("vCPU Debugger", "Input memory location to look at (ex. 0xF000 or 4096):", "0x1000");
            if(input != NULL) {
                uint16_t addr = (uint16_t)strtol(input, NULL, 0);
                uint8_t val = cpu.memory[addr];

                char msg[128];
                sprintf(msg, "Address: 0x%0X\nValue: 0x%02X (%d)", addr, val, val);
                tinyfd_messageBox("Memory Inspector", msg, "ok", "info", 1);
            }
        }

        if(IsKeyPressed(KEY_E)) {
            const char* input = tinyfd_inputBox("vCPU Debugger", "How much instructions to step (Step X): ", "10");
            if(input != NULL) {
                int steps = atoi(input);
                if (steps > 0) {
                    running = false;
                    stepReq += steps;
                }
            }
        }

        if(IsKeyPressed(KEY_W)) {
            const char* input = tinyfd_inputBox("vCPU Debugger", "Input memory location to poke (ex. 0xF000 or 4096):", "0x1000");
            if(input != NULL) {
                uint16_t addr = (uint16_t)strtol(input, NULL, 0);
                const char* valMsg = tinyfd_inputBox("Memory Poker", "Enter value to set (ex. 0xA0 or 10): ", "8");
                if(valMsg != NULL) {
                    uint8_t val = (uint8_t)strtol(valMsg, NULL, 0);
                    cpu.memory[addr] = val;
                
                    char msg[128];
                    sprintf(msg, "Address: 0x%0X\nValue: 0x%02X (%d)", addr, val, val);
                    tinyfd_messageBox("Memory Poker", msg, "ok", "info", 1);
                }
            }
        }

        if(IsKeyPressed(KEY_R)) {
            const char* input = tinyfd_inputBox("vCPU Debugger", "Where to put PC (ex. 0xF000 or 4096): ", "0x1000");
            if(input != NULL) {
                int PCval = atoi(input);
                cpu.PC = PCval;
            }
        }

        if(IsKeyPressed(KEY_P)) {
            running = !running;
        }

        if(IsKeyPressed(KEY_T)) {
            const char* input = tinyfd_inputBox("vCPU Debugger", "Enter new speed (in Hz) ", "5000");
            if(input != NULL) {
                uint64_t cpsNum = (int)strtol(input, NULL, 0);
                targetCps = cpsNum;
            }
        }
    }

    running = false;
    shouldExit = true;
    if(cpuThread.joinable()) {
        cpuThread.join();
    }
    return 0;
}
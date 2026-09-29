std::string opcodes[256];

void init_opcodes() {
    for (int i = 0; i < 256; i++) opcodes[i] = "UNK";
    opcodes[0x30] = "STA";
    opcodes[0x31] = "STX";
    opcodes[0x32] = "STY";
    opcodes[0x33] = "MAX";
    opcodes[0x34] = "MAY";
    opcodes[0x35] = "MXA";
    opcodes[0x36] = "MXY";
    opcodes[0x37] = "MYA";
    opcodes[0x38] = "MYX";
    opcodes[0x40] = "STM";
    opcodes[0x41] = "LDM";
    opcodes[0x50] = "ADD";
    opcodes[0x51] = "SUB";
    opcodes[0x52] = "MUL";
    opcodes[0x53] = "DIV";
    opcodes[0x54] = "SHR";
    opcodes[0x55] = "SHL";
    opcodes[0x56] = "AND";
    opcodes[0x57] = "OR";
    opcodes[0x58] = "XOR";
    opcodes[0x59] = "NOR";
    opcodes[0x60] = "JMP";
    opcodes[0x61] = "CALL";
    opcodes[0x62] = "RET";
    opcodes[0x70] = "CMP";
    opcodes[0x71] = "JZ";
    opcodes[0x72] = "JNZ";
    opcodes[0x73] = "JL";
    opcodes[0x74] = "JM";
    opcodes[0x80] = "CLF";
    opcodes[0x81] = "NOP";
    opcodes[0x82] = "HLT";
    opcodes[0x83] = "PUSH";
    opcodes[0x84] = "POP";
    opcodes[0x85] = "INA";
    opcodes[0x86] = "INX";
    opcodes[0x87] = "INY";
    opcodes[0x88] = "DEA";
    opcodes[0x89] = "DEX";
    opcodes[0x90] = "DEY";
    opcodes[0x91] = "REPM";
    opcodes[0x92] = "REPW";
    opcodes[0x93] = "CRD";
    opcodes[0xF0] = "SYSCALL";
}


// Registers: A X Y
// A - Math register
// X - Memory register
// Y - Other register
// 16 bit registers

// Register set (SeT _ )
#define STA 0x30 // Load A
#define STX 0x31 // Load X
#define STY 0x32 // Load Y

// Move register values
#define MAX 0x33 // A -> X
#define MAY 0x34 // A -> Y

#define MXA 0x35 // X -> A
#define MXY 0x36 // X -> Y

#define MYA 0x37 // Y -> A
#define MYX 0x38 // Y -> X

// Memory operation
#define STM 0x40 // Store value at pointer X (value in Y)
#define LDM 0x41 // Load value at pointer X (load to Y)

// Math
#define ADD 0x50 // Add A with Y
#define SUB 0x51 // Subtract A with Y
#define MUL 0x52 // Multiply A with Y
#define DIV 0x53 // Divide A with Y (rest in X)

// Bitwise operations
#define SHR 0x54 // Shift A right Y bits
#define SHL 0x55 // Shift A left Y bits

#define AND 0x56 // AND A with Y
#define OR 0x57 // OR A with Y
#define XOR 0x58 // XOR A with Y
#define NOR 0x59 // NOR A with Y

#define JMP 0x60 // Move PC to value
#define CALL 0x61 // Push PC to stack and jump
#define RET 0x62 // Get PC from stack and return

#define CMP 0x70 // Compare A and Y
#define JZ 0x71 // Jump if zero
#define JNZ 0x72 // Jump if not zero
#define JL 0x73 // Jump if less
#define JM 0x74 // Jump if more

#define CLF 0x80 // Clear flags

#define NOP 0x81 // Do nothing
#define HLT 0x82 // Halt with code A

#define PUSH 0x83 // Push Y into the stack
#define POP 0x84 // Pop the stack into Y

#define INA 0x85
#define INX 0x86
#define INY 0x87
#define DEA 0x88
#define DEX 0x89
#define DEY 0x90

#define REPM 0x91
#define REPW 0x92

#define CRD 0x93

#define SYSCALL 0xF0

#define VRAM_BEGIN 0xF000 // VRAM beginning
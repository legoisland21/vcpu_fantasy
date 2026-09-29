import sys

OPCODES_NO_ARGS = {
    "NOP": 0x81,
    "HLT": 0x82,
    "CLF": 0x80,
    "RET": 0x62,
    "MAX": 0x33, 
    "MAY": 0x34,
    "MXA": 0x35,
    "MXY": 0x36,
    "MYA": 0x37,
    "MYX": 0x38,
    "STM": 0x40,
    "LDM": 0x41,
    "PUSH": 0x83,
    "POP": 0x84,
    "INA": 0x85,
    "INX": 0x86,
    "INY": 0x87,
    "DEA": 0x88,
    "DEX": 0x89,
    "DEY": 0x90,
    "ADD": 0x50,
    "SUB": 0x51,
    "MUL": 0x52,
    "DIV": 0x53,
    "SHR": 0x54,
    "SHL": 0x55,
    "AND": 0x56,
    "OR":  0x57,
    "XOR": 0x58,
    "NOR": 0x59,
    "CMP": 0x70,
    "REPM": 0x91,
    "REPW": 0x92,
    "CRD": 0x93,
    "SYS": 0xF0,
}

OPCODES_ARGS = {
    "STA": 0x30,
    "STX": 0x31,
    "STY": 0x32,
    "JMP": 0x60,
    "CALL": 0x61,
    "JZ":  0x71,
    "JNZ": 0x72,
    "JL":  0x73,
    "JM":  0x74,
}

def parseVal(val, labels):
    if val in labels:
        return labels[val]

    if val.startswith("%") and len(val) == 2:
        return ord(val[1])
    
    valL = val.lower()
    
    if valL.startswith("0x"):
        return int(val[2:], 16)
    if valL.endswith("h"):
        return int(val[:-1], 16)
    if valL.startswith("0b"):
        return int(val[2:], 2)
    if valL.endswith("b"):
        return int(val[:-1], 2)
    
    return int(val)

def extractTextContent(raw_args_str):
    raw = raw_args_str.strip()
    if (raw.startswith("'") and raw.endswith("'")) or (raw.startswith('"') and raw.endswith('"')):
        raw = raw[1:-1]
    raw = raw.encode('utf-8').decode('unicode_escape')
    return raw

def assemble(input, output):
    with open(input, "r") as f:
        lines = f.readlines()

    labels = {}
    BASE_ADDR = 0x1000

    address = BASE_ADDR
    cleanLines = []

    for line in lines:
        line = line.split(";")[0].strip()
        if not line:
            continue

        if line.endswith(":"):
            labelName = line[:-1].strip()
            labels[labelName] = address
            continue

        first_space = line.find(" ")
        if first_space == -1:
            mnemonic = line.upper()
            raw_args = ""
        else:
            mnemonic = line[:first_space].strip().upper()
            raw_args = line[first_space:].strip()

        if mnemonic == ".TEXT":
            text_str = extractTextContent(raw_args)
            cleanLines.append((mnemonic, [text_str]))
            address += len(text_str) + 1
            continue

        parts = line.replace(",", " ").split()
        mnemonic = parts[0].upper()
        args = parts[1:]

        cleanLines.append((mnemonic, args))

        if mnemonic in OPCODES_NO_ARGS:
            address += 1
        elif mnemonic in OPCODES_ARGS:
            address += 3
        elif mnemonic == ".BYTE":
            address += len(args)
        elif mnemonic == ".WORD":
            address += len(args) * 2
        else:
            print(f"Unknown instruction '{mnemonic}'")
            sys.exit(1)
        
    binaryData = bytearray()

    for mnemonic, args in cleanLines:
        if mnemonic in OPCODES_NO_ARGS:
            binaryData.append(OPCODES_NO_ARGS[mnemonic])
        elif mnemonic in OPCODES_ARGS:
            opcode = OPCODES_ARGS[mnemonic]
            val = parseVal(args[0], labels) & 0xFFFF

            hb = (val >> 8) & 0xFF
            lb = val & 0xFF

            binaryData.append(opcode)
            binaryData.append(hb)
            binaryData.append(lb)
        elif mnemonic == ".BYTE":
            for arg in args:
                val = parseVal(arg, labels) & 0xFF
                binaryData.append(val)
        elif mnemonic == ".WORD":
            for arg in args:
                val = parseVal(arg, labels) & 0xFFFF
                binaryData.append((val >> 8) & 0xFF)
                binaryData.append(val & 0xFF)
        elif mnemonic == ".TEXT":
            text_str = args[0]
            for char in text_str:
                binaryData.append(ord(char) & 0xFF)
            binaryData.append(0x00)

    with open(output, "wb") as f:
        f.write(binaryData)

    print(f"Compilation completed at {len(binaryData)} bytes! Thank you for using vMASM.")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python vmasm.py program.vas out.vex")
    else:
        assemble(sys.argv[1], sys.argv[2])
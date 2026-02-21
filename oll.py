#!/usr/bin/env python3
# OLL - Own Little Language

import sys

###########################
#    Tokenize Program     #
###########################


def tokenize(filepath: str):
    program: list[tuple[str, int | str | None]] = []
    labels = {}
    pc: int = 0

    with open(filepath, "r") as f:
        for raw_line in f:
            line = raw_line.strip()
            if not line:
                continue

            parts = line.split()
            opcode = parts[0].upper()

            # label
            if opcode.endswith(":"):
                labels[opcode[:-1]] = pc
                continue

            if opcode == "PUSH":
                program.append(("PUSH", int(parts[1])))
            elif opcode == "PRINT":
                string_literal = " ".join(parts[1:])[1:-1]
                program.append(("PRINT", string_literal))
            elif opcode in ("JUMP.EQ.0", "JUMP.GT.0"):
                program.append((opcode, parts[1]))
            elif opcode == "HALT":
                code = int(parts[1]) if len(parts) > 1 else 0
                program.append(("HALT", code))
            else:
                program.append((opcode, None))

            pc += 1

    return program, labels


###########################
#          Stack          #
###########################


class Stack:
    def __init__(self, size=256):
        self.buf = []
        self.size = size

    def push(self, value):
        if len(self.buf) >= self.size:
            raise RuntimeError("Stack overflow")
        self.buf.append(value)

    def pop(self):
        if not self.buf:
            raise RuntimeError("Stack underflow")
        return self.buf.pop()

    def top(self):
        if not self.buf:
            raise RuntimeError("Stack underflow")
        return self.buf[-1]


###########################
#    Interpret Program    #
###########################


def run(program, labels) -> int:
    pc = 0
    stack = Stack()

    while pc < len(program):
        opcode, operand = program[pc]
        opcode = opcode.upper() if opcode else None
        pc += 1

        if opcode and (opcode[0] == ";" or opcode[0] == "#!"): # comment or shebang
            continue

        if opcode == "PUSH":
            stack.push(operand)

        elif opcode == "POP":
            stack.pop()

        elif opcode == "ADD":
            b = stack.pop()
            a = stack.pop()
            stack.push(a + b)

        elif opcode == "SUB":
            b = stack.pop()
            a = stack.pop()
            stack.push(a - b)

        elif opcode == "PRINT":
            print(operand)

        elif opcode == "READ":
            stack.push(int(input()))

        # ===== CONDITIONAL JUMPS (BACKWARD-COMPATIBLE) =====

        elif opcode == "JUMP.EQ.0":
            value = stack.top()
            if value == 0:
                stack.pop()  # consume ONLY if jump taken
                pc = labels[operand]

        elif opcode == "JUMP.GT.0":
            value = stack.top()
            if value > 0:
                pc = labels[operand]  # NEVER consume

        elif opcode == "HALT":
            return operand

        else:
            raise RuntimeError(f"Unknown opcode: {opcode}")

    return 0


###########################
#            Main         #
###########################

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: oll.py <program> [--dump]")
        sys.exit(1)

    program, labels = tokenize(sys.argv[1])

    if len(sys.argv) > 2 and sys.argv[2] == "--dump":
        print(program)
    else:
        sys.exit(run(program, labels))

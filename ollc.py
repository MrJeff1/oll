#!/usr/bin/env python3
import subprocess
import sys

# =====================
# Instruction definition
# =====================


class Op:
    PUSH = "PUSH"
    POP = "POP"
    ADD = "ADD"
    SUB = "SUB"
    READ = "READ"
    PRINT = "PRINT"
    JEQ0 = "JEQ0"
    JGT0 = "JGT0"
    CALL = "CALL"
    RET = "RET"
    HALT = "HALT"


class Instr:
    def __init__(self, op, arg=0, target=0, string=None):
        self.op = op
        self.arg = arg
        self.target = target
        self.string = string


# =====================
# Helpers
# =====================


def trim(s: str) -> str:
    return s.strip(" \t\r\n")


# =====================
# Main
# =====================


def main():
    if len(sys.argv) < 2:
        print("usage: ollc <file.oll>", file=sys.stderr)
        sys.exit(1)

    # ---- Read source ----
    with open(sys.argv[1], "r", newline=None) as f:
        src = f.read()

    instructions = []
    labels = {}
    functions = {}

    # ---- First pass ----
    for raw_line in src.split("\n"):
        line = trim(raw_line)
        if not line:
            continue

        # label
        if line.endswith(":"):
            label = trim(line[:-1])
            labels[label] = len(instructions)
            continue

        parts = line.split()
        op = parts[0]
        op = op.upper()
        
        if op.startswith(";") or op.startswith("#!"):  # comment or shebang
            continue  # directive, ignore for now

        if op == "FUNC":
            functions[parts[1]] = len(instructions)

        elif op == "PUSH":
            instructions.append(Instr(Op.PUSH, arg=int(parts[1])))

        elif op == "PRINT":
            rest = line[len("PRINT") :].strip()
            if rest.startswith('"') and rest.endswith('"'):
                rest = rest[1:-1]
            instructions.append(Instr(Op.PRINT, string=rest))

        elif op == "CALL":
            instructions.append(Instr(Op.CALL, string=parts[1]))

        elif op == "RET":
            instructions.append(Instr(Op.RET))

        elif op == "JUMP.EQ.0":
            instructions.append(Instr(Op.JEQ0, string=parts[1]))

        elif op == "JUMP.GT.0":
            instructions.append(Instr(Op.JGT0, string=parts[1]))

        elif op == "ADD":
            instructions.append(Instr(Op.ADD))

        elif op == "SUB":
            instructions.append(Instr(Op.SUB))

        elif op == "POP":
            instructions.append(Instr(Op.POP))

        elif op == "READ":
            instructions.append(Instr(Op.READ))

        elif op == "HALT":
            instructions.append(Instr(Op.HALT))

    # ---- Resolve targets ----
    for ins in instructions:
        if ins.op == Op.CALL:
            if ins.string not in functions:
                print(f"undefined function: {ins.string}", file=sys.stderr)
                sys.exit(1)
            ins.target = functions[ins.string]
        elif ins.op in (Op.JEQ0, Op.JGT0):
            if ins.string not in labels:
                print(f"undefined label: {ins.string}", file=sys.stderr)
                sys.exit(1)
            ins.target = labels[ins.string]

    # ---- Emit C ----
    with open("out.c", "w") as out:
        out.write(
            "#include <stdio.h>\n"
            "int stack[256], callstack[256];\n"
            "int sp=-1, csp=-1;\n"
            "int main(){int pc=0;while(1){switch(pc){\n"
        )

        for i, ins in enumerate(instructions):
            out.write(f"case {i}:\n")

            if ins.op == Op.PUSH:
                out.write(f"stack[++sp]={ins.arg}; pc++; break;\n")
            elif ins.op == Op.POP:
                out.write("sp--; pc++; break;\n")
            elif ins.op == Op.ADD:
                out.write("stack[sp-1]+=stack[sp]; sp--; pc++; break;\n")
            elif ins.op == Op.SUB:
                out.write("stack[sp-1]-=stack[sp]; sp--; pc++; break;\n")
            elif ins.op == Op.READ:
                out.write('scanf("%d", &stack[++sp]); pc++; break;\n')
            elif ins.op == Op.PRINT:
                out.write(f'puts("{ins.string}"); pc++; break;\n')
            elif ins.op == Op.JEQ0:
                out.write(f"if(stack[sp--]==0) pc={ins.target}; else pc++; break;\n")
            elif ins.op == Op.JGT0:
                out.write(f"if(stack[sp]>0) pc={ins.target}; else pc++; break;\n")
            elif ins.op == Op.CALL:
                out.write(f"callstack[++csp]=pc+1; pc={ins.target}; break;\n")
            elif ins.op == Op.RET:
                out.write("pc=callstack[csp--]; break;\n")
            elif ins.op == Op.HALT:
                out.write("return 0;\n")

        out.write("}}}\n")

    # ---- Compile ----
    subprocess.check_call(["cc", "out.c", "-Oz", "-o", "a.out"])
    print("emitted out.c and built a.out")


if __name__ == "__main__":
    main()

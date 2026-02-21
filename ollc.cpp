#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>

using namespace std;

struct B
{
    static constexpr const char *PUSH = "PUSH";
    static constexpr const char *POP = "POP";
    static constexpr const char *ADD = "ADD";
    static constexpr const char *SUB = "SUB";
    static constexpr const char *READ = "READ";
    static constexpr const char *PRINT = "PRINT";
    static constexpr const char *JEQ0 = "JEQ0";
    static constexpr const char *JGT0 = "JGT0";
    static constexpr const char *CALL = "CALL";
    static constexpr const char *RET = "RET";
    static constexpr const char *HALT = "HALT";
};

struct F
{
    string op;
    int arg;
    int target;
    string string_val;
    F(const string &op_, int arg_ = 0, int target_ = 0, const string &string_val_ = "")
        : op(op_), arg(arg_), target(target_), string_val(string_val_) {}
};

string P(const string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

int main(int argc, char *argv[])
{
    const char *X = "HALT";
    const char *W = "RET";
    const char *V = "CALL";
    const char *U = "READ";
    const char *T = "SUB";
    const char *S = "ADD";
    const char *R = "POP";
    const char *Q = "PUSH";
    const char *M = "PRINT";

    if (argc < 2)
    {
        cerr << "usage: ollc <file.oll>" << endl;
        return 1;
    }

    ifstream infile(argv[1]);
    if (!infile)
    {
        cerr << "Failed to open file: " << argv[1] << endl;
        return 1;
    }

    string b((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
    infile.close();

    vector<F> C;
    unordered_map<string, int> N;
    unordered_map<string, int> O;

    istringstream iss(b);
    string line;
    while (getline(iss, line))
    {
        string I = P(line);
        if (I.empty())
            continue;
        if (I.back() == ':')
        {
            string d = P(I.substr(0, I.size() - 1));
            N[d] = (int)C.size();
            continue;
        }
        istringstream line_ss(I);
        vector<string> H;
        string word;
        while (line_ss >> word)
        {
            H.push_back(word);
        }
        if (H.empty())
            continue;
        string D = H[0];
        if (D[0] == ';' || (D.size() > 1 && D[0] == '#' && D[1] == '!'))
            continue;
        if (D == "FUNC")
        {
            if (H.size() > 1)
                O[H[1]] = (int)C.size();
        }
        else if (D == Q)
        {
            if (H.size() > 1)
                C.emplace_back(B::PUSH, stoi(H[1]));
        }
        else if (D == M)
        {
            string J = I.substr(strlen(M));
            J = P(J);
            if (J.size() >= 2 && J.front() == '"' && J.back() == '"')
            {
                J = J.substr(1, J.size() - 2);
            }
            C.emplace_back(B::PRINT, 0, 0, J);
        }
        else if (D == V)
        {
            if (H.size() > 1)
                C.emplace_back(B::CALL, 0, 0, H[1]);
        }
        else if (D == W)
        {
            C.emplace_back(B::RET);
        }
        else if (D == "JUMP.EQ.0")
        {
            if (H.size() > 1)
                C.emplace_back(B::JEQ0, 0, 0, H[1]);
        }
        else if (D == "JUMP.GT.0")
        {
            if (H.size() > 1)
                C.emplace_back(B::JGT0, 0, 0, H[1]);
        }
        else if (D == S)
        {
            C.emplace_back(B::ADD);
        }
        else if (D == T)
        {
            C.emplace_back(B::SUB);
        }
        else if (D == R)
        {
            C.emplace_back(B::POP);
        }
        else if (D == U)
        {
            C.emplace_back(B::READ);
        }
        else if (D == X)
        {
            C.emplace_back(B::HALT);
        }
    }

    for (auto &A : C)
    {
        if (A.op == B::CALL)
        {
            if (O.find(A.string_val) == O.end())
            {
                cerr << "undefined function: " << A.string_val << endl;
                return 1;
            }
            A.target = O[A.string_val];
        }
        else if (A.op == B::JEQ0 || A.op == B::JGT0)
        {
            if (N.find(A.string_val) == N.end())
            {
                cerr << "undefined label: " << A.string_val << endl;
                return 1;
            }
            A.target = N[A.string_val];
        }
    }

    const char *Y = "out.c";
    ofstream E(Y);
    if (!E)
    {
        cerr << "Failed to open output file: " << Y << endl;
        return 1;
    }

    E << "#include <stdio.h>\nint stack[256], callstack[256];\nint sp=-1, csp=-1;\nint main(){int pc=0;while(1){switch(pc){\n";
    for (size_t e = 0; e < C.size(); ++e)
    {
        const F &A = C[e];
        E << "case " << e << ":\n";
        if (A.op == B::PUSH)
        {
            E << "stack[++sp]=" << A.arg << "; pc++; break;\n";
        }
        else if (A.op == B::POP)
        {
            E << "sp--; pc++; break;\n";
        }
        else if (A.op == B::ADD)
        {
            E << "stack[sp-1]+=stack[sp]; sp--; pc++; break;\n";
        }
        else if (A.op == B::SUB)
        {
            E << "stack[sp-1]-=stack[sp]; sp--; pc++; break;\n";
        }
        else if (A.op == B::READ)
        {
            E << "scanf(\"%d\", &stack[++sp]); pc++; break;\n";
        }
        else if (A.op == B::PRINT)
        {
            // Escape double quotes and backslashes in string_val for C string literal
            string escaped;
            for (char ch : A.string_val)
            {
                if (ch == '\\' || ch == '"')
                {
                    escaped.push_back('\\');
                }
                escaped.push_back(ch);
            }
            E << "puts(\"" << escaped << "\"); pc++; break;\n";
        }
        else if (A.op == B::JEQ0)
        {
            E << "if(stack[sp--]==0) pc=" << A.target << "; else pc++; break;\n";
        }
        else if (A.op == B::JGT0)
        {
            E << "if(stack[sp]>0) pc=" << A.target << "; else pc++; break;\n";
        }
        else if (A.op == B::CALL)
        {
            E << "callstack[++csp]=pc+1; pc=" << A.target << "; break;\n";
        }
        else if (A.op == B::RET)
        {
            E << "pc=callstack[csp--]; break;\n";
        }
        else if (A.op == B::HALT)
        {
            E << "return 0;\n";
        }
    }
    E << "}}}\n";
    E.close();

    if (system("cc out.c -Oz -o a.out") != 0)
    {
        cerr << "Failed to compile out.c" << endl;
        return 1;
    }
    cout << "emitted out.c and built a.out" << endl;

    return 0;
}
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <cctype>
#include <variant>

using Program = std::vector<std::pair<std::string, std::variant<int, std::string, std::nullptr_t>>>;
using Labels = std::unordered_map<std::string, int>;

Program tokenize(const std::string &filepath, Labels &labels)
{
    Program program;
    int pc = 0;

    std::ifstream file(filepath);
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open file: " + filepath);
    }

    std::string raw_line;
    while (std::getline(file, raw_line))
    {
        std::string line;
        // trim
        size_t start = raw_line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos)
            continue;
        size_t end = raw_line.find_last_not_of(" \t\r\n");
        line = raw_line.substr(start, end - start + 1);
        if (line.empty())
            continue;

        std::istringstream iss(line);
        std::vector<std::string> parts;
        std::string part;
        while (iss >> part)
        {
            parts.push_back(part);
        }
        if (parts.empty())
            continue;

        std::string opcode = parts[0];
        for (auto &c : opcode)
            c = std::toupper(c);

        // label
        if (!opcode.empty() && opcode.back() == ':')
        {
            std::string label = opcode.substr(0, opcode.size() - 1);
            labels[label] = pc;
            continue;
        }

        if (opcode == "PUSH")
        {
            if (parts.size() < 2)
                throw std::runtime_error("PUSH requires an operand");
            int val = std::stoi(parts[1]);
            program.emplace_back("PUSH", val);
        }
        else if (opcode == "PRINT")
        {
            // join parts[1:] and remove first and last char (quotes)
            std::string joined;
            for (size_t i = 1; i < parts.size(); ++i)
            {
                if (i > 1)
                    joined += " ";
                joined += parts[i];
            }
            if (joined.size() < 2)
                throw std::runtime_error("PRINT operand malformed");
            std::string string_literal = joined.substr(1, joined.size() - 2);
            program.emplace_back("PRINT", string_literal);
        }
        else if (opcode == "JUMP.EQ.0" || opcode == "JUMP.GT.0")
        {
            if (parts.size() < 2)
                throw std::runtime_error(opcode + " requires a label operand");
            program.emplace_back(opcode, parts[1]);
        }
        else if (opcode == "HALT")
        {
            int code = 0;
            if (parts.size() > 1)
            {
                code = std::stoi(parts[1]);
            }
            program.emplace_back("HALT", code);
        }
        else
        {
            program.emplace_back(opcode, nullptr);
        }
        ++pc;
    }

    return program;
}

class Stack
{
public:
    Stack(size_t size = 256) : size_(size) {}

    void push(int value)
    {
        if (buf_.size() >= size_)
        {
            throw std::runtime_error("Stack overflow");
        }
        buf_.push_back(value);
    }

    int pop()
    {
        if (buf_.empty())
        {
            throw std::runtime_error("Stack underflow");
        }
        int val = buf_.back();
        buf_.pop_back();
        return val;
    }

    int top() const
    {
        if (buf_.empty())
        {
            throw std::runtime_error("Stack underflow");
        }
        return buf_.back();
    }

private:
    std::vector<int> buf_;
    size_t size_;
};

int run(const Program &program, const Labels &labels)
{
    int pc = 0;
    Stack stack;

    while (pc < (int)program.size())
    {
        const auto &[opcode, operand] = program[pc];
        ++pc;

        if (!opcode.empty() && (opcode[0] == ';' || (opcode.size() > 1 && opcode[0] == '#' && opcode[1] == '!')))
        {
            continue;
        }

        if (opcode == "PUSH")
        {
            stack.push(std::get<int>(operand));
        }
        else if (opcode == "POP")
        {
            stack.pop();
        }
        else if (opcode == "ADD")
        {
            int b = stack.pop();
            int a = stack.pop();
            stack.push(a + b);
        }
        else if (opcode == "SUB")
        {
            int b = stack.pop();
            int a = stack.pop();
            stack.push(a - b);
        }
        else if (opcode == "PRINT")
        {
            std::cout << std::get<std::string>(operand) << std::endl;
        }
        else if (opcode == "READ")
        {
            int input_val;
            std::cin >> input_val;
            stack.push(input_val);
        }
        else if (opcode == "JUMP.EQ.0")
        {
            int value = stack.top();
            if (value == 0)
            {
                stack.pop();
                auto it = labels.find(std::get<std::string>(operand));
                if (it == labels.end())
                    throw std::runtime_error("Label not found: " + std::get<std::string>(operand));
                pc = it->second;
            }
        }
        else if (opcode == "JUMP.GT.0")
        {
            int value = stack.top();
            if (value > 0)
            {
                auto it = labels.find(std::get<std::string>(operand));
                if (it == labels.end())
                    throw std::runtime_error("Label not found: " + std::get<std::string>(operand));
                pc = it->second;
            }
        }
        else if (opcode == "HALT")
        {
            return std::get<int>(operand);
        }
        else
        {
            throw std::runtime_error("Unknown opcode: " + opcode);
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: oll <program> [--dump]" << std::endl;
        return 1;
    }

    Labels labels;
    Program program;
    try
    {
        program = tokenize(argv[1], labels);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error tokenizing program: " << e.what() << std::endl;
        return 1;
    }

    if (argc > 2 && std::string(argv[2]) == "--dump")
    {
        for (const auto &[op, operand] : program)
        {
            std::cout << "(" << op << ", ";
            if (std::holds_alternative<int>(operand))
            {
                std::cout << std::get<int>(operand);
            }
            else if (std::holds_alternative<std::string>(operand))
            {
                std::cout << "\"" << std::get<std::string>(operand) << "\"";
            }
            else
            {
                std::cout << "None";
            }
            std::cout << ")" << std::endl;
        }
        return 0;
    }
    else
    {
        try
        {
            return run(program, labels);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Runtime error: " << e.what() << std::endl;
            return 1;
        }
    }
}
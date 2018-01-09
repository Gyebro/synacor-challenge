//
// Created by Gyebro on 2018. 01. 09..
//

#include <c++/sstream>
#include "vm.h"

// UTILS

union bytes_to_uint16 {
    struct {
        char low;
        char high;
    } bytes;
    uint16_t uint16;
};

uint16_t add(const uint16_t a, const uint16_t b) {
    return (uint16_t)( (int(a)+int(b))%32768 );
}

uint16_t mult(const uint16_t a, const uint16_t b) {
    return (uint16_t)( (int(a)*int(b))%32768 );
}


// MEMBER FUNCTIONS

vm::vm(string program) {
    // Initialize registers and memory
    cout << "Initializing registers and memory\n";
    for (size_t i=0; i<REGISTERS; i++)      registers[i]=0;
    for (size_t i=0; i<MEMORY_SIZE; i++)    memory[i]=0;
    // Read the program into the memory
    cout << "Reading program 'challenge.bin'\n";
    ifstream program_bin(program, ios::binary);
    char bytes[2];
    size_t c = 0;
    bytes_to_uint16 u;
    while(program_bin.read(bytes, 2)) {
        u.bytes.low = bytes[0];
        u.bytes.high = bytes[1];
        memory[c] = u.uint16;
        c++;
    }
    cout << "Program contains " << c << " uint16 values\n";
}

void vm::set_reg(const uint16_t a, const uint16_t v) {
    if (a <= 32767) {
        program_state = error;
    } else if (a <= 32775) {
        registers[(a - 32768)] = v; // Register address 0..7
    } else {
        program_state = error;
    }
}

void vm::set_mem(const uint16_t a, const uint16_t v) {
    if (a <= 32767) {
        memory[a] = v;
    } else {
        program_state = error;
    }
}

uint16_t vm::convert_value(const uint16_t value) {
    if (value <= 32767) {
        return value; // Literal
    } else if (value <= 32775) {
        return registers[(value-32768)]; // Register address 0..7
    } else {
        program_state = error;
        return 0;
    }
}

void vm::operate() {
    uint16_t a, b, c, v;
    size_t opsize = 0;
    switch (memory[memory_ptr]) {
        case 0: // halt;
            program_state = terminated;
            opsize = 1;
            break;
        case 1: // set a b
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            opsize = 3;
            set_reg(a, convert_value(b));
            break;
        case 2: // push a
            a = memory[memory_ptr+1];
            opsize = 2;
            stack.push_back(convert_value(a));
            break;
        case 3: // pop a
            a = memory[memory_ptr+1];
            opsize = 2;
            if (stack.size() == 0) program_state = error;
            set_reg(a, stack.back());
            stack.pop_back();
            break;
        case 4: // eq a = (b == c)
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            c = memory[memory_ptr+3];
            opsize = 4;
            v = (uint16_t)(convert_value(b)== convert_value(c) ? 1 : 0);
            set_reg(a, v);
            break;
        case 5: // gt a = (b > c)
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            c = memory[memory_ptr+3];
            opsize = 4;
            v = (uint16_t)(convert_value(b)> convert_value(c) ? 1 : 0);
            set_reg(a, v);
            break;
        case 6: // jmp a
            a = memory[memory_ptr+1];
            opsize = 0;
            memory_ptr = convert_value(a);
            break;
        case 7: // jt a b
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            if (convert_value(a) != 0) {
                memory_ptr = convert_value(b);
                opsize = 0; // Avoid incrementing after switch
            } else {
                opsize = 3;
            }
            break;
        case 8: // jf a b
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            if (convert_value(a) == 0) {
                memory_ptr = convert_value(b);
                opsize = 0; // Avoid incrementing after switch
            } else {
                opsize = 3;
            }
            break;
        case 9: // add a b c
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            c = memory[memory_ptr+3];
            opsize = 4;
            set_reg(a, add(convert_value(b), convert_value(c)));
            break;
        case 10: // mult a b c
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            c = memory[memory_ptr+3];
            opsize = 4;
            set_reg(a, mult(convert_value(b), convert_value(c)));
            break;
        case 11: // mod a b c
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            c = memory[memory_ptr+3];
            opsize = 4;
            set_reg(a, convert_value(b) % convert_value(c));
            break;
        case 12: // and a b c
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            c = memory[memory_ptr+3];
            opsize = 4;
            set_reg(a, convert_value(b) & convert_value(c));
            break;
        case 13: // or a b c
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            c = memory[memory_ptr+3];
            opsize = 4;
            set_reg(a, convert_value(b) | convert_value(c));
            break;
        case 14: // not a b
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            opsize = 3;
            v = (uint16_t)((~convert_value(b)) & 0x7fff); // 15-bit bitwise inverse
            set_reg(a, v);
            break;
        case 15: // rmem a b TESTED
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            opsize = 3;
            set_reg(a, memory[convert_value(b)]);
            break;
        case 16: // wmem a b
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            opsize = 3;
            set_mem(convert_value(a), convert_value(b));
            break;
        case 17: // call a TESTED
            a = memory[memory_ptr+1];
            opsize = 0; // Avoid extra jump
            stack.push_back((uint16_t)(memory_ptr+2));
            memory_ptr = convert_value(a);
            break;
        case 18: // ret
            opsize = 0; // Avoid extra jump
            if (stack.size() == 0) program_state = error;
            v = stack.back();
            stack.pop_back();
            memory_ptr = v;
            break;
        case 19: // out a
            a = memory[memory_ptr+1];
            opsize = 2;
            out << (char) convert_value(a);
            break;
        case 20: // in a
            a = memory[memory_ptr+1];
            opsize = 2;
            if (input.empty()) {
                program_state = waiting_for_input;
                return; // TODO: Proper halt and continue
                v = (uint16_t)getchar();
            } else {
                v = (uint16_t)input.front(); input.pop_front();
                out.str(""); // TODO: control this
            }
            set_reg(a, v);
            break;
        case 21: // noop
            opsize = 1;
            break;
        default:
            opsize = 1;
            break;

    }
    memory_ptr += opsize;
}

void vm::run_program(uint16_t ptr) {
    memory_ptr = ptr;
    stack.resize(0);
    program_state = running;
    while(program_state == running) {
        operate();
    }
}

string vm::get_strings() {
    stringstream ss;
    memory_ptr = 0;
    uint16_t opsize;
    while(memory_ptr < MEMORY_SIZE) {
        switch (memory[memory_ptr]) {
            case 0: // halt;
            case 18: // ret
            case 21: // noop
                opsize = 1;
                break;
            case 2: // push a
            case 3: // pop a
            case 6: // jmp a
            case 17: // call a
            case 20: // in a
                opsize = 2;
                break;
            case 1: // set a b
            case 7: // jt a b
            case 8: // jf a b
            case 14: // not a b
            case 15: // rmem a b
            case 16: // wmem a b
                opsize = 3;
                break;
            case 4: // eq a = (b == c)
            case 5: // gt a = (b > c)
            case 9: // add a b c
            case 10: // mult a b c
            case 11: // mod a b c
            case 12: // and a b c
            case 13: // or a b c
                opsize = 4;
                break;
            case 19: // out a
                opsize = 2;
                ss << (char)convert_value(memory[memory_ptr+1]);
                break;
            default:
                opsize = 1;
                break;
        }
        memory_ptr+=opsize;
    }
    return ss.str();
}

void vm::add_input(string line) {
    for (char c : line) {
        input.push_back(c);
    }
}

const string coins[] = {"red", "corroded", "shiny", "concave", "blue"};
vector<int> values = {2, 3, 5, 7, 9};

void permute(vector<int> a, int i, int n, vector<int>& s) {
    int j;
    if (i == n) {
        if (a[0]+a[1]*a[2]*a[2]+a[3]*a[3]*a[3]-a[4] == 399) {
            s = a;
        }
    } else {
        for (j = i; j <= n; j++) {
            swap(a[i], a[j]);
            permute(a, i+1, n, s);
            swap(a[i], a[j]);
        }
    }
}

void vm::solve_coin_problem() {
    vector<int> solution;
    permute(values, 0, 4, solution);
    for (int v : solution) {
        for (size_t i=0; i<values.size(); i++) {
            if (values[i] == v) add_input("use "+coins[i]+" coin\n");
        }
    }
}

string vm::get_output(bool clear) {
    string output = out.str();
    if (clear) out.str("");
    return output;
}

string vm::print_registers() {
    string out = "R:V\n";
    size_t i=0;
    for (uint16_t v : registers) {
        out += to_string(i++) + ":" + to_string(v) + "\n";
    }
    return out;
}
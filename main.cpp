#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

#define MEMORY_SIZE 32768
#define REGISTERS 8

enum state {
    running,
    terminated,
    error,
    waiting_for_input
};

uint16_t memory[MEMORY_SIZE];
uint16_t memory_ptr;
uint16_t registers[REGISTERS];
vector<uint16_t> stack;
state program_state;

union bytes_to_uint16 {
    struct {
        char low;
        char high;
    } bytes;
    uint16_t uint16;
};

void set_value(const uint16_t a, const uint16_t v) {
    if (v > 32767) {
        cout << "Invalid value!\n";
    }
    if (a <= 32767) {
        memory[a] = v;
        return;
    } else if (a <= 32775) {
        registers[(a-32768)] = v; // Register address 0..7
        return;
    } else {
        program_state = error;
    }
}

uint16_t read_value(const uint16_t value) {
    if (value <= 32767) {
        return value; // Literal
    } else if (value <= 32775) {
        return registers[(value-32768)]; // Register address 0..7
    } else {
        program_state = error;
        return 0;
    }
}

uint16_t add(const uint16_t a, const uint16_t b) {
    return (uint16_t)( (int(a)+int(b))%32768 );
}

uint16_t mult(const uint16_t a, const uint16_t b) {
    return (uint16_t)( (int(a)*int(b))%32768 );
}

void operate() {
    uint16_t a, b, c, v;
    size_t opsize = 0;
    //cout << "mptr: " << memory_ptr << endl;
    flush(cout);
    switch (memory[memory_ptr]) {
        case 0: // halt;
            program_state = terminated;
            opsize = 1;
            break;
        case 1: // set a b
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            opsize = 3;
            set_value(a, read_value(b));
            break;
        case 2: // push a
            a = memory[memory_ptr+1];
            opsize = 2;
            stack.push_back(read_value(a));
            break;
        case 3: // pop a
            a = memory[memory_ptr+1];
            opsize = 2;
            set_value(a, stack.back());
            stack.pop_back();
            break;
        case 4: // eq a = (b == c)
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            c = memory[memory_ptr+3];
            opsize = 4;
            v = (uint16_t)(read_value(b)==read_value(c) ? 1 : 0);
            set_value(a, v);
            break;
        case 5: // gt a = (b > c)
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            c = memory[memory_ptr+3];
            opsize = 4;
            v = (uint16_t)(read_value(b)>read_value(c) ? 1 : 0);
            set_value(a, v);
            break;
        case 6: // jmp a
            a = memory[memory_ptr+1];
            opsize = 0;
            memory_ptr = read_value(a);
            break;
        case 7: // jt a b
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            if (read_value(a) != 0) {
                memory_ptr = read_value(b);
                opsize = 0; // Avoid incrementing after switch
            } else {
                opsize = 3;
            }
            break;
        case 8: // jf a b
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            if (read_value(a) == 0) {
                memory_ptr = read_value(b);
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
            set_value(a, add(read_value(b), read_value(c)));
            break;
        case 10: // mult a b c
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            c = memory[memory_ptr+3];
            opsize = 4;
            set_value(a, mult(read_value(b), read_value(c)));
            break;
        case 11: // mod a b c
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            c = memory[memory_ptr+3];
            opsize = 4;
            set_value(a, read_value(b)%read_value(c));
            break;
        case 12: // and a b c
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            c = memory[memory_ptr+3];
            opsize = 4;
            set_value(a, read_value(b)&read_value(c));
            break;
        case 13: // or a b c
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            c = memory[memory_ptr+3];
            opsize = 4;
            set_value(a, read_value(b)|read_value(c));
            break;
        case 14: // not a b
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            opsize = 3;
            v = (uint16_t)((~read_value(b)) & 0x7fff); // 15-bit bitwise inverse
            set_value(a, v);
            break;
        case 15: // rmem a b
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            opsize = 3;
            set_value(a, b);
            break;
        case 16: // wmem a b
            a = memory[memory_ptr+1];
            b = memory[memory_ptr+2];
            opsize = 3;
            set_value(a, read_value(b));
            break;
        case 17: // call a
            a = memory[memory_ptr+1];
            opsize = 0; // Avoid extra jump
            stack.push_back((uint16_t)(memory_ptr+2));
            memory_ptr = read_value(a);
            break;
        case 18: // ret
            opsize = 0; // Avoid extra jump
            v = stack.back();
            stack.pop_back();
            memory_ptr = v;
            break;
        case 19: // out a
            a = memory[memory_ptr+1];
            opsize = 2;
            cout << (char)read_value(a);
            break;
        case 20: // in a
            a = memory[memory_ptr+1];
            opsize = 2;
            v = (uint16_t)getchar();
            set_value(a, v);
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

void run_program(uint16_t ptr) {
    memory_ptr = ptr;
    stack.resize(0);
    while(program_state == running) {
        operate();
    }
}

int main() {
    // Initialize registers and memory
    cout << "Initializing registers and memory\n";
    for (size_t i=0; i<REGISTERS; i++)      registers[i]=0;
    for (size_t i=0; i<MEMORY_SIZE; i++)    memory[i]=0;
    // Read the program into the memory
    cout << "Reading program 'challenge.bin'\n";
    ifstream program_bin("challenge.bin", ios::binary);
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
    // Run the program
    run_program(0);

    getchar();

    return 0;
}
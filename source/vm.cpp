//
// Created by Gyebro on 2018. 01. 09..
//

#include <c++/sstream>
#include <c++/thread>
#include "vm.h"
#include "confirmation.h"

#define NUM_THREADS 8
#define RANGE 32768/NUM_THREADS

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

void vm::operate(bool terminal_interaction) {
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
            if (terminal_interaction) cout << (char) convert_value(a);
            break;
        case 20: // in a
            a = memory[memory_ptr+1];
            opsize = 2;
            if (input.empty()) {
                program_state = waiting_for_input;
                if (terminal_interaction) {
                    v = (uint16_t)getchar();
                    program_state = running;
                } else {
                    return;
                }
            } else {
                v = (uint16_t)input.front(); input.pop_front();
                if (!terminal_interaction) out.str("");
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

void vm::run_program(uint16_t ptr, bool terminal_interaction) {
    memory_ptr = ptr;
    stack.resize(0);
    program_state = running;
    while(program_state == running) {
        operate(terminal_interaction);
    }
}

const string opstrings[] = {"halt","set","push","pop","eq","gt","jmp","jt","jf","add","mult",
                            "mod","and","or","not","rmem","wmem","call","ret","out","in","noop"};

void vm::resume_program(bool terminal_interaction) {
    vector<breakpoint> empty(0);
    resume_program(empty, terminal_interaction);
}

void vm::resume_program(vector<breakpoint>& breakpoints, bool terminal_interaction) {
    program_state = running;
    bool breakpoint_hit = false;
    string breakpoint_text;
    while(program_state == running) {
        // Check breakpoints
        for (const breakpoint& b : breakpoints) {
            if (b.op > 21) {
                // Break at line
                if (memory_ptr == b.op) {
                    breakpoint_hit = true;
                    breakpoint_text = "break at line: " + memory_ptr;
                }
            }
            if (!breakpoint_hit && memory[memory_ptr] == b.op) {
                switch (b.t) {
                    case break_at_op:
                        breakpoint_hit = true;
                        breakpoint_text = "break at op";
                        break;
                    case break_at_op_on_a:
                        if (memory[memory_ptr+1] == b.arg) {
                            breakpoint_hit = true;
                            breakpoint_text = "break at op if 'a'=="+to_string(b.arg);
                        }
                        break;
                    case break_at_op_on_b:
                        if (memory[memory_ptr+2] == b.arg) {
                            breakpoint_hit = true;
                            breakpoint_text = "break at op if 'b'=="+to_string(b.arg);
                        }
                        break;
                    case break_at_op_on_c:
                        if (memory[memory_ptr+3] == b.arg) {
                            breakpoint_hit = true;
                            breakpoint_text = "break at op if 'c'=="+to_string(b.arg);
                        }
                        break;
                    case break_at_op_val_a:
                        if (convert_value(memory[memory_ptr+1]) == b.arg) {
                            breakpoint_hit = true;
                            breakpoint_text = "break at op if val('a')=="+to_string(b.arg);
                        }
                        break;
                }
            }
            if (breakpoint_hit) {
                cout << "Breakpoint hit!\n";
                if (b.op <= 21 ) cout << opstrings[b.op];
                cout << " type: " << breakpoint_text << endl;
                program_state = terminated;
                break;
            }
        }
        if (!breakpoint_hit) operate(terminal_interaction);
    }
}

void vm::step_one() {
    operate();
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

uint16_t vm::get_memory_ptr() const {
    return memory_ptr;
}

void vm::set_memory_ptr(uint16_t p) {
    memory_ptr = p;
}

uint16_t *vm::get_memory() {
    return memory;
}

uint16_t *vm::get_registers() {
    return registers;
}

vector<uint16_t> vm::get_stack() const {
    return stack;
}

state vm::get_state() const {
    return program_state;
}

string vm::get_operation_text(uint16_t ptr) {
    uint16_t a, b, c, v;
    switch (memory[ptr]) {
        case 0: // halt;
            return "halt";
        case 1: // set a b
            a = memory[ptr+1]-32768;
            b = convert_value(memory[ptr+2]);
            return "set <" + to_string(a) +"> to " + to_string(b);
        case 2: // push a
            a = convert_value(memory[ptr+1]);
            return "push " + to_string(a);
        case 3: // pop a
            a = convert_value(memory[ptr+1]);
            return "pop into <" + to_string(a) +"> " + to_string(stack.back());
        case 4: // eq a = (b == c)
            a = memory[ptr+1]-32768;
            b = convert_value(memory[ptr+2]);
            c = convert_value(memory[ptr+3]);
            return "eq into <" + to_string(a) + "> " + to_string(b) + " == " + to_string(c);
        case 5: // gt a = (b > c)
            a = memory[ptr+1]-32768;
            b = convert_value(memory[ptr+2]);
            c = convert_value(memory[ptr+3]);
            return "gt into <" + to_string(a) + "> " + to_string(b) + " > " + to_string(c);
        case 6: // jmp a
            a = convert_value(memory[ptr+1]);
            return "jmp to " + to_string(a);
        case 7: // jt a b
            a = convert_value(memory[ptr+1]);
            b = convert_value(memory[ptr+2]);
            return "jt if <" + to_string(memory[ptr+1]-32768) + ">=" + to_string(a) + " != 0 to " + to_string(b);
        case 8: // jf a b
            a = convert_value(memory[ptr+1]);
            b = convert_value(memory[ptr+2]);
            return "jt if <" + to_string(memory[ptr+1]-32768) + ">=" + to_string(a) + " == 0 to " + to_string(b);
        case 9: // add a b c
            a = memory[ptr+1]-32768;
            b = convert_value(memory[ptr+2]);
            c = convert_value(memory[ptr+3]);
            return "add into <" + to_string(a) + "> " + to_string(b) + " + " + to_string(c);
        case 10: // mult a b c
            a = memory[ptr+1]-32768;
            b = convert_value(memory[ptr+2]);
            c = convert_value(memory[ptr+3]);
            return "mult into <" + to_string(a) + "> " + to_string(b) + " * " + to_string(c);
        case 11: // mod a b c
            a = memory[ptr+1]-32768;
            b = convert_value(memory[ptr+2]);
            c = convert_value(memory[ptr+3]);
            return "mod into <" + to_string(a) + "> " + to_string(b) + " % " + to_string(c);
        case 12: // and a b c
            a = memory[ptr+1]-32768;
            b = convert_value(memory[ptr+2]);
            c = convert_value(memory[ptr+3]);
            return "and into <" + to_string(a) + "> " + to_string(b) + " & " + to_string(c);
        case 13: // or a b c
            a = memory[ptr+1]-32768;
            b = convert_value(memory[ptr+2]);
            c = convert_value(memory[ptr+3]);
            return "or into <" + to_string(a) + "> " + to_string(b) + " | " + to_string(c);
        case 14: // not a b
            a = memory[ptr+1]-32768;
            b = convert_value(memory[ptr+2]);
            return "not into <" + to_string(a) + "> ~" + to_string(b);
        case 15: // rmem a b TESTED
            a = memory[ptr+1]-32768;
            b = convert_value(memory[ptr+2]);
            return "rmem into <" + to_string(a) + "> from addr " + to_string(b) + " (value: " + to_string(memory[b]) + ")";
        case 16: // wmem a b
            a = convert_value(memory[ptr+1]);
            b = convert_value(memory[ptr+2]);
            return "wmem into addr " + to_string(a) + " value " + to_string(b);
        case 17: // call a
            a = convert_value(memory[ptr+1]);
            return "call " + to_string(a) + " (push " + to_string(ptr+2) + " into stack)";
        case 18: // ret
            v = stack.back();
            return "ret " + to_string(v);
        case 19: // out a
        {
            a = convert_value(memory[ptr+1]);
            string str = "out ' '";
            if (a == 10) {
                return "out '\\n'";
            }
            str[5] = char(a);
            return str;
        }
        case 20: // in a
            a = memory[ptr+1]-32768;
            return "in into <" + to_string(a) + ">";
        case 21: // noop
            return "noop";
        default:
            return "-";
    }
}

void find_input(int tid) {
    const uint16_t result = 6;
    confirmation c;
    uint16_t min = (tid-1)*RANGE;
    uint16_t max = (tid)*RANGE;
    uint16_t input = c.get_input_for_output(result, min, max);
    if (input > 0) {
        cout << "Thread " << tid;
        cout << " found! r[7]=" << input << " yields func6027().r[0]=6\n";
    }
}

uint16_t vm::solve_confirmation_problem() {
    cout << "Finding confirmation routine input\n";
    thread t[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        t[i] = thread(find_input, i);
    }
    cout << "Launched " << NUM_THREADS << " threads\n";
    for (int i = 0; i < NUM_THREADS; i++) {
        t[i].join();
    }
    return 0;

}

enum class maze_tile_type {
    value,
    plus,
    minus,
    mult
};

struct maze_tile {
    size_t id;
    maze_tile_type type;
    int value;
    vector<size_t> neighbours;
};

class maze {
private:
    vector<maze_tile> tiles;
    size_t t_start, t_end;
    int weight(const vector<size_t> path) {
        if (path.front()!=0) {
            return -1;
        }
        if (path.size()%2==0) {
            return -1; // Error
        }
        int weight = tiles[0].value;
        for (size_t i=1; i<path.size(); i+=2) {
            int val = tiles[path[i+1]].value;
            switch (tiles[path[i]].type) {
                case maze_tile_type::value:
                    return -1; // Error
                case maze_tile_type::plus:
                    weight += val;
                    break;
                case maze_tile_type::minus:
                    weight -= val;
                    break;
                case maze_tile_type::mult:
                    weight *= val;
                    break;
            }
        }
        return weight;
    }
public:
    maze() {
        tiles.resize(16);
        t_start = 0;
        t_end = 15;
    }
    void add(const maze_tile& t) {
        tiles[t.id]=t;
    }
    void update_neighbours() {
        for (size_t i=0; i<16; i++) {
            vector<size_t> n;
            if (i > 3)  n.push_back(i-4); // South
            if (i < 12) n.push_back(i+4); // North
            if (i % 4 != 0) n.push_back(i-1); // West
            if (i % 4 != 3) n.push_back(i+1); // East
            tiles[i].neighbours=n;
        }
    }
    vector<size_t> bfs(size_t target_value) {
        vector<vector<size_t>> paths, new_paths;
        paths.push_back({t_start});
        size_t depth = 0;
        vector<size_t> path;
        int max_weight = 70;
        while (true) {
            new_paths.resize(0);
            new_paths.reserve(paths.size()*4);
            for (size_t i=0; i<paths.size(); i++) {
                for (size_t n : tiles[paths[i].back()].neighbours) {
                    path = paths[i];
                    path.push_back(n);
                    new_paths.push_back(path);
                }
            }
            paths = new_paths;
            depth++;
            int w;
            size_t erase_start = 0;
            size_t erase_shatter = 0;
            size_t erase_end = 0;
            size_t erase_too_heavy = 0;
            if (depth % 2 == 0) {
                for (size_t i=0; i<paths.size(); i++) {
                    // Discard those who went back to t_start
                    if (paths[i].back()==t_start) {
                        paths.erase(paths.begin()+i); i--;
                        erase_start++;
                    } else {
                        // Calculate orb weight
                        w = weight(paths[i]);
                        // Discard 0 or negative weights
                        if (w <= 0) {
                            paths.erase(paths.begin()+i); i--;
                            erase_shatter++;
                        } else if (w >= max_weight) {
                            paths.erase(paths.begin()+i); i--;
                            erase_too_heavy++;
                        } else if (paths[i].back() == t_end) {
                            // At the end tile
                            if (w == target_value) {
                                cout << "Orb value at vault: " << w << endl;
                                return paths[i];
                            } else {
                                // Discard those whose value does not match the destination
                                paths.erase(paths.begin()+i); i--;
                                erase_end++;
                            }
                        }
                    }
                }
                cout << "BFS: depth=" << depth << ", paths=" << paths.size() << endl;
                cout << " Orb shattered in " << erase_shatter << " cases\n";
                cout << " Orb was reset in " << erase_start << " cases\n";
                cout << " Orb was too heavy in " << erase_too_heavy << " cases\n";
                cout << " Orb failed to open vault in " << erase_end << " cases\n";
            }
        }
    }
};

/**
 * Maze layout
 * [ * ]-[ 8 ]-[ - ]-[ 1 ]
 *   |     |     |     |
 * [ 4 ]-[ * ]-[11 ]-[ * ]
 *   |     |     |     |
 * [ + ]-[ 4 ]-[ - ]-[18 ]
 *   |     |     |     |
 * [22 ]-[ - ]-[ 9 ]-[ * ]
 */
void vm::solve_maze_problem() {
    maze m;
    vector<size_t> n(0);
    m.add({ 0, maze_tile_type::value, 22, n});
    m.add({ 1, maze_tile_type::minus,  0, n});
    m.add({ 2, maze_tile_type::value,  9, n});
    m.add({ 3, maze_tile_type::mult,   0, n});

    m.add({ 4, maze_tile_type::plus,   0, n});
    m.add({ 5, maze_tile_type::value,  4, n});
    m.add({ 6, maze_tile_type::minus,  0, n});
    m.add({ 7, maze_tile_type::value, 18, n});

    m.add({ 8, maze_tile_type::value,  4, n});
    m.add({ 9, maze_tile_type::mult,   0, n});
    m.add({10, maze_tile_type::value, 11, n});
    m.add({11, maze_tile_type::mult,   0, n});

    m.add({12, maze_tile_type::mult,   0, n});
    m.add({13, maze_tile_type::value,  8, n});
    m.add({14, maze_tile_type::minus,  0, n});
    m.add({15, maze_tile_type::value,  1, n});

    m.update_neighbours();

    vector<size_t> path = m.bfs(30);

    cout << "Solution found: steps=" << path.size()-1 << endl;

    for (size_t i=0; i<path.size()-1; i++) {
        int i0 = path[i];
        int i1 = path[i+1];
        switch (i1-i0) {
            case +1:
                add_input("east\n");
                break;
            case -1:
                add_input("west\n");
                break;
            case +4:
                add_input("north\n");
                break;
            case -4:
                add_input("south\n");
                break;
            default:
                cout << "Error while processing path\n";
                break;
        }
    }

    return;
}

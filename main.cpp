#include "vm.h"
#include "confirmation.h"
#include <cstdlib>
#include <iomanip>
#include <algorithm>

template<typename T>
void split(const std::string &s, char delim, T result) {
    stringstream ss(s); string item;
    while (getline(ss, item, delim)) {
        *(result++) = item;
    }
}

vector<string> split(const string &s, char delim=' ') {
    vector<string> elems;
    split(s, delim, back_inserter(elems));
    return elems;
}

void print_help() {
    cout << "\nDebugger commands:\n";
    cout << "o:        view last output\n";
    cout << "q:        quit debugging\n";
    cout << "i [text]: send input\n";
    cout << "c:        continue execution\n";
    cout << "s:        step one\n";
    cout << "r n v:    sets register n to v\n";
    cout << "b op key val: add breakpoint\n";
}

void print_screen(vm& p) {
    uint16_t* registers = p.get_registers();
    uint16_t* memory = p.get_memory();
    uint16_t  memory_ptr = p.get_memory_ptr();
    uint16_t  addr = 0;
    cout << "+-----------+---------------+-----------\n";
    cout << "| Registers | Memory        | Operation \n";
    cout << "+-----------+---------------+-----------\n";
    uint16_t lookback = 2;
    uint16_t lines = 32;
    for (uint16_t i=0; i<lines; i++) {
        if (i < 8) {
            cout << "| " << i << " = " << setw(5) << registers[i] << " |";
        } else if (i == 8) {
            cout << "+-----------+";
        } else {
            cout << "            |";
        }
        if (memory_ptr+i >= lookback) {
            addr = memory_ptr+i-lookback;
            cout << (addr==memory_ptr?"[":" ") << setw(5) << addr << (addr==memory_ptr?"]":" ")
                 << "= " << setw(5) << memory[addr] << " |";
        } else {
            cout << "               |";
        }
        cout << " " << p.get_operation_text(addr);
        cout << endl;
    }
}

void clear_screen() {
    if (system("CLS")) system("clear");
}

int main() {

    confirmation routine(1);

    vm program("challenge.bin");

    // Add input to avoid typing
    program.add_input("take tablet\n"
              "use tablet\n" /* Code 4 */
              "doorway\n"
              "north\n"
              "north\n"
              "bridge\n"
              "continue\n"
              "down\n"
              "east\n"
              "take empty lantern\n"
              "west\n"
              "west\n"
              "passage\n"
              "ladder\n"
              "west\n"
              "south\n"
              "north\n" /* Code 5 */
              "take can\n"
              "use can\n"
              "use lantern\n"
              "west\n"
              "ladder\n"
              "darkness\n"
              "continue\n"
              "west\nwest\nwest\nwest\n"
              "north\n"
              "take red coin\n"
              "north\n"
              "west\n"
              "take blue coin\n"
              "up\n"
              "take shiny coin\n"
              "down\neast\neast\n"
              "take concave coin\n"
              "down\n"
              "take corroded coin\n"
              "up\nwest\n"
    );

    program.solve_coin_problem();

    program.add_input("north\n"
              "take teleporter\n"
              "use teleporter\n" /* Code 6 */
              "take business card\n"
              "take strange book\n"
    );

    // Run the program
    program.run_program(0);

    // Enter debug loop
    bool debugging = true;
    string line;
    vector<string> words;
    uint16_t reg, val;
    vector<breakpoint> breakpoints;
    breakpoint bp;
    while (debugging) {
        clear_screen();
        print_screen(program);
        print_help();
        getline(cin, line);
        words = split(line);
        cout << "Command: " << line << endl;
        switch (line[0]) {
            case 'o':
                cout << program.get_output() << endl;
                cout << "Press [ENTER] to continue debugging\n"; getchar();
                break;
            case 'i':
                program.add_input(line.substr(2)+"\n");
                break;
            case 'c':
                program.resume_program(breakpoints);
                break;
            case 's':
                program.step_one();
                break;
            case 'r':
                reg = (uint16_t)stoul(words[1]);
                val = (uint16_t)stoul(words[2]);
                program.get_registers()[reg] = val;
                break;
            case 'b':
                // b op key val
                bp.op = (uint16_t)stoul(words[1]);
                bp.arg = 0;
                switch (words[2][0]) {
                    case '0':
                        bp.t = break_at_op;
                        break;
                    case '1':
                        bp.t = break_at_op_on_a;
                        bp.arg = (uint16_t)stoul(words[3]);
                        break;
                    case '2':
                        bp.t = break_at_op_on_b;
                        bp.arg = (uint16_t)stoul(words[3]);
                        break;
                    case '3':
                        bp.t = break_at_op_on_c;
                        bp.arg = (uint16_t)stoul(words[3]);
                        break;
                    case '4':
                        bp.t = break_at_op_val_a;
                        bp.arg = (uint16_t)stoul(words[3]);
                        break;
                }
                breakpoints.push_back(bp);
                break;
            case 'q':
                debugging = false;
                break;
        }
    }
    return 0;
}
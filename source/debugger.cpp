//
// Created by Gyebro on 2018. 01. 12..
//

#include "debugger.h"
#include <iomanip>

void debugger::print_help() {
    cout << "\nDebugger commands:\n";
    cout << "o:        view last output\n";
    cout << "i [text]: send input\n";
    cout << "c:        continue execution\n";
    cout << "s:        step one\n";
    cout << "r n v:    sets register n to v\n";
    cout << "g n:      go to address n\n";
    cout << "b op key val: add breakpoint\n";
    cout << "p:        patch/bypass confirmation\n";
    cout << "q:        quit debugging and continue execution\n";
    cout << "x:        quit debugging and shut down\n";
}

void debugger::print_screen() {
    uint16_t* registers = p->get_registers();
    uint16_t* memory = p->get_memory();
    uint16_t  memory_ptr = p->get_memory_ptr();
    uint16_t  addr = 0;
    cout << "+-----------+---------------+-----------\n";
    cout << "| Registers | Memory        | Operation \n";
    cout << "+-----------+---------------+-----------\n";
    uint16_t lookback = 2;
    uint16_t lines = 32;
    for (uint16_t i=0; i<lines; i++) {
        if (i < 8) {
            cout << "| " << i << " = " << setw(5) << registers[i] << " |";
        } else if (i == 8 || i == 10) {
            cout << "+-----------+";
        } else if (i == 9) {
            cout << "+ Stack top +";
        } else if (i == 11) {
            cout << "| s = " << setw(5) << p->get_stack().size() << " |";
        } else  {
            if (i-12 < p->get_stack().size() && (i-12 < 10)) {
                cout << "|-" << i-12 << " = " << setw(5) << p->get_stack()[p->get_stack().size()-(i-12)-1] << " |";
            } else {
                cout << "            |";
            }
        }
        if (memory_ptr+i >= lookback) {
            addr = memory_ptr+i-lookback;
            cout << (addr==memory_ptr?"[":" ") << setw(5) << addr << (addr==memory_ptr?"]":" ")
                 << "= " << setw(5) << memory[addr] << " |";
        } else {
            cout << "               |";
        }
        cout << " " << p->get_operation_text(addr);
        cout << endl;
    }
}

void debugger::attach(vm *program_pointer) {
    p = program_pointer;
    // Enter debug loop
    bool debugging = true;
    string line;
    vector<string> words;
    uint16_t reg, val;
    vector<breakpoint> breakpoints;
    breakpoint bp;
    while (debugging) {
        clear_screen();
        print_screen();
        print_help();
        flush(cout);
        getline(cin, line);
        words = split(line);
        cout << "Command: " << line << endl;
        switch (line[0]) {
            case 'o':
                cout << p->get_output() << endl;
                cout << "Press [ENTER] to continue debugging\n"; getchar();
                break;
            case 'i':
                p->add_input(line.substr(2)+"\n");
                break;
            case 'c':
                p->resume_program(breakpoints);
                break;
            case 's':
                p->step_one();
                break;
            case 'r':
                reg = (uint16_t)stoul(words[1]);
                val = (uint16_t)stoul(words[2]);
                p->get_registers()[reg] = val;
                break;
            case 'g':
                p->set_memory_ptr(stoul(words[1]));
                break;
            case 'l':
                bp.op = (uint16_t)stoul(words[1]);
                breakpoints.push_back(bp);
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
            case 'p':
                // Patch confirmation routine
                p->add_input("use teleporter\n");
                p->get_registers()[7] = 25734;
                bp.op = 5489;
                breakpoints.push_back(bp);
                p->resume_program(breakpoints);
                p->get_registers()[0] = 6;
                p->set_memory_ptr(5491);
                p->resume_program(breakpoints);
                break;
            case 'q':
                debugging = false;
                break;
            case 'x':
                return;
        }
    }
    return;
}

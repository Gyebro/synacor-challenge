//
// Created by Gyebro on 2018. 01. 12..
//

#ifndef SYNACOR_CHALLENGE_DEBUGGER_H
#define SYNACOR_CHALLENGE_DEBUGGER_H


#include "vm.h"
#include "utils.h"

class debugger {
private:
    vm* p;
    void print_help();
    void print_screen();
public:
    debugger() {
    }
    void attach(vm* program_pointer);
};


#endif //SYNACOR_CHALLENGE_DEBUGGER_H

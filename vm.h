//
// Created by Gyebro on 2018. 01. 09..
//

#ifndef SYNACOR_CHALLENGE_VM_H
#define SYNACOR_CHALLENGE_VM_H

#include <vector>
#include <deque>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

#define MEMORY_SIZE 32768
#define REGISTERS 8

enum state {
    running,
    terminated,
    error,
    waiting_for_input
};

class vm {
private:
    uint16_t memory[MEMORY_SIZE];
    uint16_t memory_ptr;
    uint16_t registers[REGISTERS];
    vector<uint16_t> stack;
    state program_state;
    deque<char> input;
    void set_reg(const uint16_t a, const uint16_t v);
    void set_mem(const uint16_t a, const uint16_t v);
    uint16_t convert_value(const uint16_t value);
    void operate();
    stringstream out;
public:
    vm(string program);
    void run_program(uint16_t ptr);
    string get_strings();
    void add_input(string line);
    void solve_coin_problem();
    string get_output(bool clear = true);
    string print_registers();
};


#endif //SYNACOR_CHALLENGE_VM_H

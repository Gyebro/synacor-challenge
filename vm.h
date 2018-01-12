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

enum breakpoint_type {
    break_at_op,
    break_at_op_on_a,
    break_at_op_on_b,
    break_at_op_on_c,
    break_at_op_val_a
};

struct breakpoint {
    uint16_t op;
    uint16_t arg;
    breakpoint_type t;
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
    void operate(bool terminal_interaction = false);
    stringstream out;
public:
    vm(string program);
    void run_program(uint16_t ptr);
    void resume_program(vector<breakpoint>& b, bool terminal_interaction = false);
    void step_one();
    void add_input(string line);
    void solve_coin_problem();
    string get_output(bool clear = false);
    uint16_t get_memory_ptr() const;
    uint16_t* get_memory();
    uint16_t* get_registers();
    void set_memory_ptr(uint16_t p);
    vector<uint16_t> get_stack() const;
    state get_state() const;
    string get_operation_text(uint16_t ptr);
    uint16_t solve_confirmation_problem();
    void solve_maze_problem();
};


#endif //SYNACOR_CHALLENGE_VM_H

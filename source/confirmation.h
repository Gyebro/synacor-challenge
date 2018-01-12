//
// Created by Gyebro on 2018. 01. 10..
//

#ifndef SYNACOR_CHALLENGE_CONFIRMATION_H
#define SYNACOR_CHALLENGE_CONFIRMATION_H


#include <c++/cstdint>
#include <vector>
#include <c++/iostream>
#include <c++/map>

using namespace std;

typedef pair<uint16_t, uint16_t> regs;


class confirmation {
private:
    uint16_t reg7;
    regs reg01;
    map<regs, regs> memo;
    uint16_t add(const uint16_t a, const uint16_t b) {
        return (uint16_t)((a+b)%32768);
    }
    uint16_t minusone(const uint16_t a) {
        return (uint16_t)((a+32767)%32768);
    }
    regs func6027(const regs r) {
        if (memo.count(r) > 0) {
            return memo[r];
        }
        /*if (r.first >= 32768) {
            cout << "ERROR in reg0!\n";
        }
        if (r.second >= 32768) {
            cout << "ERROR in reg1!\n";
        }*/
        if (r.first == 0) {
            regs ret = make_pair(add(r.second,1), r.second);
            memo[r]=ret;
            return ret;
        }
        if (r.second == 0) {
            regs ret = func6027( make_pair(minusone(r.first), reg7) );
            memo[r]=ret;
            return ret;
        }
        uint16_t new_reg1 = func6027( make_pair(r.first, minusone(r.second)) ).first;
        regs ret = func6027( make_pair(minusone(r.first), new_reg1) );
        memo[r]=ret;
        return ret;
    }
public:
    confirmation() : reg7(0) {
        reg01 = regs(4, 1);
    }
    uint16_t run(uint16_t eight_register) {
        reg7 = eight_register;
        memo.clear();
        return func6027(reg01).first;
    }
    uint16_t get_input_for_output(uint16_t result, uint16_t min, uint16_t max) {
        for (reg7 = min; reg7 < max; reg7++) {
            if (result == run(reg7)) return reg7;
        }
        return 0;
    }
};


#endif //SYNACOR_CHALLENGE_CONFIRMATION_H

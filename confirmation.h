//
// Created by Gyebro on 2018. 01. 10..
//

#ifndef SYNACOR_CHALLENGE_CONFIRMATION_H
#define SYNACOR_CHALLENGE_CONFIRMATION_H


#include <c++/cstdint>
#include <vector>
#include <c++/iostream>

using namespace std;

class confirmation {
private:
    vector<uint16_t> stack;
    uint16_t reg0, reg1, reg7;
    void func6027() {
        if (reg0 == 0) {
            reg0 = (reg1+1)%32768;
            return;
        }
        if (reg1==0) {
            reg0 = (reg0+32767)%32768;
            reg1 = reg7;
            func6027();
            return;
        }
        stack.push_back(reg0);
        reg1 = (reg1+32767)%32768;
        func6027();
        reg1 = reg0;
        reg0 = stack.back(); stack.pop_back();
        reg0 = (reg0+32767)%32768;
        func6027();
        return;
    }
public:
    confirmation(uint16_t reg7) : reg7(reg7) {
        stack.resize(0);
        reg0 = 4;
        reg1 = 1;
        func6027();
        cout << reg0 << endl;
    }
};


#endif //SYNACOR_CHALLENGE_CONFIRMATION_H

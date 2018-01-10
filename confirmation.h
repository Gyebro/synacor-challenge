//
// Created by Gyebro on 2018. 01. 10..
//

#ifndef SYNACOR_CHALLENGE_CONFIRMATION_H
#define SYNACOR_CHALLENGE_CONFIRMATION_H


#include <c++/cstdint>
#include <vector>
#include <c++/iostream>

using namespace std;

struct regs {
	uint16_t reg0;
	uint16_t reg1;
};

class confirmation {
private:
    uint16_t reg7;
    regs func6027(regs r) {
        if (r.reg0 == 0) {
			r.reg0 = r.reg1+1;	//reg0 = (reg1+1)%32768;
            return r;
        }
        if (r.reg1 == 0) {
			r.reg0--;			//reg0 = (reg0+32767)%32768;
            r.reg1 = reg7;
            return func6027(r);
        }
        //uint16_t temp = reg0;
		r.reg1--;				//reg1 = (reg1+32767)%32768;
        r.reg1 = func6027(r).reg0;
        //reg1 = reg0;
        //reg0 = temp;
		r.reg0--;				//reg0 = (reg0+32767)%32768;
        return func6027(r);
    }
public:
    confirmation(uint16_t reg7) : reg7(reg7) {
		regs r = {4, 1};
        cout << func6027(r).reg0;
    }
};


#endif //SYNACOR_CHALLENGE_CONFIRMATION_H

//
// Created by Gyebro on 2018. 01. 12..
//

#ifndef SYNACOR_CHALLENGE_UTILS_H
#define SYNACOR_CHALLENGE_UTILS_H

#include <string>
#include <c++/sstream>
#include <c++/vector>

using namespace std;

template<typename T>
void split(const string &s, char delim, T result) {
    stringstream ss(s); string item;
    while (getline(ss, item, delim)) {
        *(result++) = item;
    }
}

vector<string> split(const string &s, char delim=' ');

void clear_screen();

#endif //SYNACOR_CHALLENGE_UTILS_H

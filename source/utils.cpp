//
// Created by Gyebro on 2018. 01. 12..
//

#include "utils.h"

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, back_inserter(elems));
    return elems;
}

void clear_screen() {
    if (system("CLS")) system("clear");
}
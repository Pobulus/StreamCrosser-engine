#pragma once
#include "Output.hpp"
#include <iostream>

using namespace std;


class StreamOutput : public Output {
public:
    StreamOutput(string name): Output(name) {  }
    bool hasStructure() override {
        return true;
    }
};



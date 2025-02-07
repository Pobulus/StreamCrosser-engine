#pragma once
#include "IProcessingNode.hpp"
#include <set>

class Set : public IProcNode {

    std::function<void (json)> _callback;
    set<json> inputs;

public:
    Set(std::string type) {

    }
    void dataInput(json data, int inputID) override {
        std::copy(data.begin(),data.end(),std::inserter(inputs,inputs.end()));
       // inputs.insert(inputs.end(), data.begin(), data.end());
        if(_callback) _callback(inputs);
    }
    void attachOutput(std::function<void (json)> callback) override {
        _callback = callback;
        callback(inputs);
    }
};


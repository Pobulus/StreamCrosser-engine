
#pragma once
#include "IProcessingNode.hpp"

class Constant : public IProcNode {

  std::function<void(json)> _callback;
  json _value;
  vector<json> output;

public:
  Constant(json value) : _value(value) {}
  void dataInput(json data, int inputID) override {}
  void attachOutput(std::function<void(json)> callback) override {
    _callback = callback;
    callback(_value);
  }
};

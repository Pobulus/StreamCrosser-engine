
#pragma once
#include "IProcessingNode.hpp"

class Prop : public IProcNode {

  vector<function<void(json)>> _callbacks;
  string _value;
  vector<json> output;
  void sendData() {
    vector<json> wrapper;
    wrapper.push_back(output);
    for (auto _callback : _callbacks)
      _callback(wrapper);
  }

public:
  Prop(std::string value) : _value(value) { output.push_back(_value); }
  void dataInput(json data, int inputID) override {
    if (data.is_object())
      return;
    output.insert(output.begin(), data);
    if (_callbacks.size())
      sendData();
  }
  void attachOutput(std::function<void(json)> callback) override {
    _callbacks.push_back(callback);
    if (output.size() > 1)
      sendData();
  }
};

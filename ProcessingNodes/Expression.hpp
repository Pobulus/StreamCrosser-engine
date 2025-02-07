#pragma once
#include "IProcessingNode.hpp"

class Expression : public IProcNode {

  json leftValue, rightValue;
  vector<function<void(json)>> _callbacks;
  string operation;

  void _callback(json data) {
    for (auto cb : _callbacks)
      cb(data);
  }

  void pushExpression() {
    if (!_callbacks.size())
      throw EngineException("Dangling node: FILTER");
    json output = json::parse("[]");
    if (leftValue.is_array())
      output = leftValue;
    else
      output.push_back(leftValue);
    if (rightValue.is_array())
      output.insert(output.end(), rightValue.begin(), rightValue.end());
    else if (operation != "NOT")
      output.push_back(rightValue);
    json op = json::parse("{}");
    op["op"] = operation;
    output.push_back(op);
    _callback(output);
  }

public:
  Expression(string value) : operation(value) {}
  void dataInput(json data, int inputID) override {
    switch (inputID) {
    case 0:
      leftValue = data;
      if ((!rightValue.empty() || operation == "NOT") && _callbacks.size())
        pushExpression();
      break;
    case 1:
      rightValue = data;
      if (!leftValue.empty() && _callbacks.size())
        pushExpression();
      break;
    }
  }
  void attachOutput(std::function<void(json)> callback) override {
    _callbacks.push_back(callback);
    if ((!rightValue.empty() || operation == "NOT") && !leftValue.empty())
      pushExpression();
  }
};

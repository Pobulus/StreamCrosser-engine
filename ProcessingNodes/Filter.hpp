#pragma once
#include "ExpressionCompiler.hpp"
#include "IProcessingNode.hpp"

class Filter : public IProcNode {

  json expression, inputData;
  vector<function<void(json)>> _callbacks;
  function<bool(json)> _filter;
  void _callback(json data) {
    for (auto cb : _callbacks) {
      cb(data);
    }
  }

  void compileExpression() {
    const json head = inputData["head"];
    _filter = ExpressionCompiler::compileExpression(expression, head);
  }
  void filterInput() {
    if (!_callbacks.size())
      throw EngineException("Dangling node: FILTER");
    compileExpression();
    vector<json> rows = {};
    json output;
    output["head"] = inputData["head"];
    output["key"] = inputData["key"];
    for (auto row : inputData["data"]) {
      if (_filter(row))
        rows.push_back(row);
    }
    output["data"] = rows;
    _callback(output);
  }

public:
  Filter() {}
  void dataInput(json data, int inputID) override {
    if (!data.is_object() && !data.is_array())
      return;

    switch (inputID) {
    case 0:
      expression = data;
      if (!inputData.empty())
        filterInput();
      break;
    case 1:
      inputData = data;
      if (!expression.empty())
        filterInput();
      break;
    }
  }
  void attachOutput(std::function<void(json)> callback) override {
    _callbacks.push_back(callback);
  }
};

#pragma once
#include "ExpressionCompiler.hpp"
#include "IProcessingNode.hpp"

class Join : public IProcNode {
  bool allRight = false;
  bool allLeft = false;
  json leftData, rightData, expression, rightKeyDef;
  function<bool(json)> condition;
  std::vector<function<void(json)>> _callbacks;
  void _callback(json data) {
    for (auto cb : _callbacks) {
      cb(data);
    }
  }
  string getKeyValue(json row) {
    string out;
    for (int k : rightData["key"]) {
      out += "+" + to_string(row[k]);
    }
    return out;
  }
  void joinInputs() {
    if (!_callbacks.size())
      throw EngineException("Dangling node: JOIN");
    vector<json> rows = {};
    const int leftLen = leftData["head"].size();
    const int rightLen = rightData["head"].size();
    vector<json> jointHead;
    jointHead.insert(jointHead.end(), leftData["head"].begin(),
                     leftData["head"].end());
    jointHead.insert(jointHead.end(), rightData["head"].begin(),
                     rightData["head"].end());
    json output;
    output["head"] = jointHead;
    output["key"] = leftData["key"];
    for (int k : rightData["key"]) {
      output["key"].push_back(leftLen + k); // increase key index
    }
    if (!condition) {
      condition =
          (expression.empty())
              ? [](json row) { return true; } // default condition - always true
              : ExpressionCompiler::compileExpression(expression, jointHead);
    }
    map<string, bool> rightFoundMap;
    // Inner join part
    for (auto leftRow : leftData["data"]) {
      bool leftFound = false;
      for (auto rightRow : rightData["data"]) {
        vector<json> jointRow;
        jointRow.insert(jointRow.end(), leftRow.begin(), leftRow.end());
        jointRow.insert(jointRow.end(), rightRow.begin(), rightRow.end());
        if (condition(jointRow)) {
          rows.push_back(jointRow);
          leftFound = true;
          rightFoundMap[getKeyValue(rightRow)] = true;
        }
      }
      if (allLeft && !leftFound) {
        vector<json> justLeft(rightLen, nullptr); // fill with nulls
        justLeft.insert(justLeft.begin(), leftRow.begin(), leftRow.end());
        rows.push_back(justLeft);
      }
    }

    // append unmatched rows from right rows if needed
    if (allRight) {
      for (auto rightRow : rightData["data"]) {
        auto key = getKeyValue(rightRow);
        if (!rightFoundMap[key]) {
          vector<json> justRight(leftLen, nullptr); // fill with nulls
          justRight.insert(justRight.end(), rightRow.begin(), rightRow.end());
          rows.push_back(justRight);
        }
      }
    }
    output["data"] = rows;
    _callback(output);
  }

public:
  Join(std::string type) {
    if (type == "Right Join")
      allRight = true;
    else if (type == "Left Join")
      allLeft = true;
    else if (type == "Full Join") {
      allRight = true;
      allLeft = true;
    }
  }
  void dataInput(json data, int inputID) override {
    if (!data.is_object() && !data.is_array())
      return;
    switch (inputID) {
    case 0:
      leftData = data;
      if (!rightData.empty())
        joinInputs();
      break;
    case 1:
      rightData = data;
      if (!leftData.empty())
        joinInputs();
      break;
    case 2:
      expression = data;
      break;
    }
  }
  void attachOutput(std::function<void(json)> callback) override {
    _callbacks.push_back(callback);
  }
};

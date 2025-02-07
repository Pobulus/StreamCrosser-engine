#pragma once
#include "EngineExceptions.hpp"
#include "IInputNode.hpp"
#include <fstream>
#include <iostream>

using namespace std;

class SourceTable : public IInputNode {

  string _name;

  vector<json> columns;
  vector<int> indexes, key;
  std::vector<function<void(json)>> _callbacks;
  void _callback(json data) {
    for (auto cb : _callbacks) {
      cb(data);
    }
  }

public:
  SourceTable(YAML::Node config) {
    _name = config["name"].as<string>();
    cout << "NODE:" << _name << endl;
    for (int i = 0; i < config["schema"].size(); i++) {
      auto column = config["schema"][i];
      vector<string> prop = {_name};
      prop.push_back(column["name"].as<string>());
      columns.push_back(prop);
      if (column["key"]) {
        key.push_back(i); // add column index as part of the key
        cout << "*";
      }
      cout << column["name"].as<string>() << endl;
    }
    cout << endl;
    indexes.insert(indexes.begin(), key.begin(), key.end());
  }
  void attachOutput(function<void(json)> callback) override {
    _callbacks.push_back(callback);
    callback(_name);
  }
  void selectProp(string prop) override {
    vector<string> multi = {_name, prop};
    auto it = find(columns.begin(), columns.end(), multi);
    if (it == columns.end())
      throw EngineException("Prop not in table definition:" +
                            ((json)multi).dump());
    indexes.push_back(it - columns.begin());
  }
  void selectAllProps() override {
    cout << "selected all columns from " << _name << endl;
    indexes.clear();
  }
  void dataInput(json data, int inputID = 0) override {
    if (!_callbacks.size())
      return;
    std::ifstream ifs("tables/" + _name + ".json");
    json fileData = json::parse(ifs);
    std::cout << fileData << endl;
    // TODO: handle json parsing errors properly
    json output;
    output["name"] = _name;
    output["key"] = key;
    if (not indexes.size()) {
      // no selection - return every colum
      output["head"] = columns;
      output["data"] = fileData;
      cout << "table sending all columns: " << output << endl;
      return _callback(output);
    }

    vector<json> headerRow = {};
    for (auto index : indexes) {
      headerRow.push_back(columns[index]);
    }
    output["head"] = headerRow;

    vector<json> rows;
    for (auto row : fileData) {
      vector<json> selectedRow = {};
      for (auto index : indexes) {
        selectedRow.push_back(row[index]);
      }
      rows.push_back(selectedRow);
    }
    output["data"] = rows;
    cout << "table sending projection" << output << endl;
    return _callback(output);
  }
  void reset() override {
    _callbacks.clear();
    selectAllProps();
  }
};

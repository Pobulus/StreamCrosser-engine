#pragma once
#include "IProcessingNode.hpp"
#include <iostream>

using namespace std;

json projection(json data, json structure) {
        vector<json> rows;
        vector<int> indexes;

        json columns = data["head"];

        for(auto str : structure) {
            auto it = find(columns.begin(), columns.end(), str);
            if(it == columns.end()) {
                cerr << "not found in head " << str <<endl;
                cerr << "head was:"  << columns << endl;
                throw EngineException("Prop not in head definition for projection");
            }
            indexes.push_back(it - columns.begin());
        }
        for(auto row : data["data"]) {
            vector<json> selectedRow = {};
            for(auto index : indexes) {
                selectedRow.push_back(row[index]);
            }
            rows.push_back(selectedRow);
        }
        return rows;
}

class Output : public IProcNode {
    json _structure = nullptr;
    string _name;
    json _data;
public:
    Output(string name) : _name(name) { }
    virtual bool hasStructure() {
        return !_structure.is_null();
    }
    void attachOutput(function<void (json)> callback) override {
        // this is a terminal node
    }
    json getData() { return _data; }
    string getName() { return _name; }

    void dataInput(json data, int inputID = 0) override {
        if(!data.is_object() && !data.is_array()) return;

        switch(inputID) {
            case 0:

                cout << "<<" << _name << ">>" << endl;
                if(_structure.is_null()) {
                    cout << data["head"] << endl << "==============" << endl;
                    for(auto line : data["data"]) { cout << line << endl; }
                    _data = data;
                    return;
                }
                _data["head"] = _structure;
                _data["key"] = json::parse("[]");
                _data["data"] = json::parse("[]");

                cout << _structure << endl << "==============" << endl;
                for(auto line : projection(data, _structure)) { cout << line << endl; _data["data"].push_back(line); }
                break;
            case 1:
                _structure = data;
                cout << "table output structure: " << _structure << endl;
                break;
        }


    }
};



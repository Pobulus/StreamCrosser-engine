#pragma once
#include "IProcessingNode.hpp"
#include "ExpressionCompiler.hpp"
#include <regex>
#include <deque>
#include <chrono>


class Window : public IProcNode {

    deque<pair<chrono::milliseconds, json>> buffer;
    vector<function<void (json)>> _callbacks;
    regex rows_regex, time_regex;
    int rowLimit = 0;
    chrono::milliseconds timeLimit;
    void _callback(json data) {
        for(auto cb : _callbacks) {
            cb(data);
        }
    }

    void setLimit(string limit) {
        smatch matches;
        cout << "Setting limit " << limit << endl;
        if (regex_search(limit, matches, rows_regex)){
            rowLimit = stoi(matches[0]);
            timeLimit = chrono::milliseconds(0);
            cout << "rows:" << rowLimit << endl;
            return;
        }
        string::const_iterator searchStart( limit.cbegin() );
        int millis = 0;
        while ( regex_search( searchStart, limit.cend(), matches, time_regex ) )
        {
            if(matches[2] == "ms") {
                millis += stoi(matches[1]);
            } else if(matches[2] == "s") {
                millis += stoi(matches[1]) * 1000;
            } else if(matches[2] == "min") {
                millis += stoi(matches[1]) * 60 * 1000;
            } else {
                cerr << "Unknown time unit!" << endl;
            }
            searchStart = matches.suffix().first;
        }
        cout << endl;
        timeLimit = chrono::milliseconds(millis);
        cout << "time limit [ms] " << millis << endl;

    }
    chrono::milliseconds millisNow() {
        return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
    }
public:
    Window(){
        // TODO: optimize by making these static
        rows_regex = regex("(\\d+)r", regex_constants::ECMAScript);
        time_regex = regex("(\\d+)(min|s|ms)", regex_constants::ECMAScript);
    }
    void dataInput(json data, int inputID=0) override {

        switch(inputID) {
            case 0:
            {
                if(!data.is_object() && !data.is_array()) return;

                auto now = millisNow();

                for(auto row : data["data"]) {
                  buffer.push_back({now, row});
                }
                if(timeLimit.count()) { // apply time limit
                    auto boundary = now - timeLimit;
                    auto it = buffer.begin();
                    while(it != buffer.end() && (*it).first < boundary){
                        buffer.pop_front();
                        it = buffer.begin();
                    }
                }
                if(rowLimit && buffer.size() > rowLimit) { // apply row limit
                    auto popCount = buffer.size() - rowLimit;
                    while(popCount > 0) {
                        buffer.pop_front();
                        popCount--;
                    }
                }
                data["data"] = json::parse("[]");
                for(auto el : buffer) {
                    data["data"].push_back(el.second);
                }
                _callback(data);
                break;
            }
            case 1:
                setLimit(data);
                break;
        }

    }
    void attachOutput(std::function<void (json)> callback) override {
        _callbacks.push_back(callback);
    }
};

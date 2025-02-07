#pragma once
#include "../includes/json.hpp"
#include <functional>
#include <stack>
#include <string>
#include <cmath>
#include "EngineExceptions.hpp"

using namespace std;
class ExpressionCompiler {
public:
    static int getId(json head, json prop){
        auto it = find(head.begin(), head.end(), prop);
        if(it == head.end()) {
            cerr << "not found in head " << prop <<endl;
            cerr << "head was" << head << endl;

            throw EngineException("Prop not in head definition during compilation");
        }
        cerr << prop << " found inside " << head <<endl;
        return (it - head.begin());
    }
    static function<json(json)> compileExpression(json expression, json head) {
        cout << "===== COMPILING EXPRESSION =====" << endl;

        stack<function<json(json)>> opstack;
        for(auto el : expression) {

            if(el.is_object()) {
                auto op = el["op"];
                auto right = opstack.top(); opstack.pop();

                if (op == "NOT") {
                    opstack.push([right](json in){ return !right(in);});
                    continue;
                }
                auto left = opstack.top(); opstack.pop();

                if (op == "==") opstack.push([left, right](json in){
                    auto l = left(in);
                    auto r = right(in);
                    return l == r;
                });
                else if (op == "!=") opstack.push([left, right](json in){ return left(in) != right(in);});
                else if (op == "<") opstack.push([left, right](json in){ return left(in) < right(in);});
                else if (op == ">") opstack.push([left, right](json in){ return left(in) > right(in);});
                else if (op == "<=") opstack.push([left, right](json in){ return left(in) <= right(in);});
                else if (op == ">=") opstack.push([left, right](json in){ return left(in) >= right(in);});
                else if (op == "AND") opstack.push([left, right](json in){ return left(in) && right(in);});
                else if (op == "OR") opstack.push([left, right](json in){ return left(in) || right(in);});
                else if (op == "+") opstack.push([left, right](json in){ return (int)left(in) + (int)right(in);});
                else if (op == "-") opstack.push([left, right](json in){ return (int)left(in) - (int)right(in);});
                else if (op == "*") opstack.push([left, right](json in){ return (int)left(in) * (int)right(in);});
                else if (op == "/") opstack.push([left, right](json in){ return (double)left(in) / (double)right(in);});
                else if (op == "//") opstack.push([left, right](json in){ return (long)left(in) / (long)right(in);});
                else if (op == "/+") opstack.push([left, right](json in){ return (long)left(in) % (long)right(in);});
                else if (op == "^") opstack.push([left, right](json in){ return pow((long)left(in), (long)right(in));});
                else if (op == "LIKE") throw EngineException("LIKE is not supported yet");
                else {throw EngineException("Unsupported operation [" + (string)op + "]");}

            } else if (el.is_array()) {
                int columnId = getId(head, el);
                opstack.push([el, columnId](json in){ return in[columnId];});
            } else {
                opstack.push([el](json in){ return el; });
            }

        }
        return opstack.top();
    }
};

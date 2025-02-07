#pragma once
#include <sstream>
#include "../includes/json.hpp"
#include <yaml-cpp/yaml.h>
#include "IProcessingNode.hpp"
#include <vector>

using json = nlohmann::json;
using namespace std;


class IInputNode : public IProcNode {
 public:
        virtual ~IInputNode() {};
        virtual void reset() = 0;
        virtual void selectProp(string prop) = 0;
        virtual void selectAllProps() = 0;

};

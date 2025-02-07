#pragma once
#include "../includes/json.hpp"
#include "IProcessingNode.hpp"
#include <yaml-cpp/yaml.h>

using json = nlohmann::json;
using namespace std;

class IInputNode : public IProcNode {
public:
  virtual ~IInputNode() {};
  virtual void reset() = 0;
  virtual void selectProp(string prop) = 0;
  virtual void selectAllProps() = 0;
};

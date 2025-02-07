#pragma once
#include "../includes/json.hpp"
#include <yaml-cpp/yaml.h>

using json = nlohmann::json;
using namespace std;

class IProcNode {
public:
  virtual ~IProcNode() {};
  virtual void attachOutput(std::function<void(json)> callback) = 0;
  virtual void dataInput(json data, int inputID = 0) = 0;
};

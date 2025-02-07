#pragma once
#include "IProcessingNode.hpp"
// TODO implementation
class Cube : public IProcNode {
public:
  void attachOutput(std::function<void(json)> callback) override {}
};

#pragma once
#include "../includes/json.hpp"
#include <algorithm>
#include <iostream>
#include <map>
#include <string>
using json = nlohmann::json;
class IdManager {
  std::map<std::string, size_t> idMap;
  size_t topId = 1;
  json diagram;

protected:
  IdManager() {}
  static IdManager *_instance;

public:
  size_t get() { return topId++; }
  void reportError(std::string message) {
    diagram["errors"].push_back(message);
  }
  size_t addSource(std::string label, std::string type) {
    if (idMap[label])
      return idMap[label]; // already defined -> don't add again
    size_t id = get();     // get new id
    idMap[label] = id;
    diagram["sources"].push_back({{"text", label}, {"type", type}, {"id", id}});
    return id;
  }
  size_t addProp(std::string name, size_t parent) {
    std::string propKey = std::to_string(parent) + "." + name;
    auto prop = diagram["props"][propKey];
    if (prop != nullptr)
      return prop;
    auto id = addNode(name, "prop");
    addNodeInput(id, parent);
    diagram["props"][propKey] = id;
    return id;
  }
  size_t addTrigger(size_t source, size_t windowSize, size_t triggerNode) {
    // size_t strm = addSource(name, "strm");
    size_t win = addNode("Window", "buff");
    size_t trig = addNode("Trigger", "emit");
    setNodeLR(win, source, windowSize);
    setNodeLR(trig, win, triggerNode);
    propToSource(triggerNode, trig, source);
    return trig;
  }
  void propToSource(size_t node, size_t parent, size_t source) {
    auto current = diagram["nodes"][std::to_string(node)];
    if (current["inputs"] == nullptr) {
      if (current["type"] == "prop") {
        // here it's necessary to create a prop node connected to the source, to
        // ensure uniqueness
        auto newProp = addProp(current["text"], source);
        addNodeInput(parent, newProp);
        // then we erase the existing (floating) prop node
        auto &vec = diagram["nodes"][std::to_string(parent)]["inputs"];
        vec.erase(std::remove(vec.begin(), vec.end(), node), vec.end());
        diagram["nodes"].erase(std::to_string(node));
      }
      return;
    }
    for (auto inp : current["inputs"])
      propToSource(inp, node, source);
  }
  size_t addAlias(std::string alias, size_t realId) {
    return idMap[alias] = realId;
  }
  size_t addNode(json value, std::string type) {
    // std::cerr << "add node" << value << " "  << type  << std::endl;
    size_t id = get();
    diagram["nodes"][std::to_string(id)] = {
        {"text", value}, {"type", type}, {"id", id}};
    return id;
  }

  void setNodeLR(size_t nodeId, size_t left, size_t right) {
    // std::cerr << "set node lr " << nodeId << " "  << left <<  " "  << right
    // << std::endl;
    auto node = diagram["nodes"].find(std::to_string(nodeId));
    if (node != diagram["nodes"].end()) {
      // simplifying the graph by assuming:
      // if 2 inputs are defined, the first one will be considered Left and the
      // second Right
      (*node)["inputs"].push_back(left);
      (*node)["inputs"].push_back(right);
    }
  }

  void addNodeInput(size_t nodeId, size_t value) {
    // std::cerr << "add node input " << nodeId << " "  << value << std::endl;
    auto node = diagram["nodes"].find(std::to_string(nodeId));
    if (node != diagram["nodes"].end()) {
      (*node)["inputs"].push_back(value);
    }
  }

  void prependNodeInput(size_t nodeId, size_t value) {
    // std::cerr << "prepend node input " << nodeId << " "  << value <<
    // std::endl;
    auto node = diagram["nodes"].find(std::to_string(nodeId));
    if (node != diagram["nodes"].end()) {
      if (!(*node)["inputs"].is_array())
        (*node)["inputs"].push_back(value);
      else
        (*node)["inputs"].insert((*node)["inputs"].begin(), value);
    }
  }

  std::string dump() {
    // std::cerr <<"id map\n";
    for (auto x : idMap) {
      // std::cerr << x.first << ": " <<x.second << std::endl;
    }
    return diagram.dump(4);
  }
  void clear() {
    idMap.clear();
    diagram = {};
    topId = 1;
  }
  IdManager(IdManager &other) = delete;
  void operator=(const IdManager &) = delete;

  static IdManager *getInstance();
};

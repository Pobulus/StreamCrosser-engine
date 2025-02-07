#pragma once
#include <exception>
#include <string>
class EngineException : std::exception {
  std::string _what;

public:
  EngineException(std::string what) : _what(what) {}
  const char *what() const noexcept override { return _what.c_str(); };
};

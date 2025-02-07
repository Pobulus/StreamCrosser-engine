#include "IdManager.hpp"

IdManager *IdManager::_instance = nullptr;
IdManager *IdManager::getInstance() {
  if (_instance == nullptr) {
    _instance = new IdManager();
  }
  return _instance;
}

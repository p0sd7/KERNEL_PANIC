#pragma once
#include <memory>
#include <stack>

#include "../DataStore/data_store.h"
#include "game_state_interface.h"

namespace kernel {

class Game {
 public:
  void Run();
  void Quit() { running_ = false; }
  void ChangeState(std::unique_ptr<GameStateInterface> new_state);
  void PushState(std::unique_ptr<GameStateInterface> new_state);
  void PopState();
  DataStore& GetDataStore() { return data_; }

 private:
  DataStore data_;
  std::stack<std::unique_ptr<GameStateInterface>> state_stack_;
  bool running_ = true;
};

}  // namespace kernel
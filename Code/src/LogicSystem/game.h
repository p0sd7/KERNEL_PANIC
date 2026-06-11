#pragma once
#include <memory>

#include "../DataStore/data_store.h"
#include "game_state_interface.h"

namespace kernel {

class Game {
 public:
  void Run();
  void Quit() { running_ = false; }
  void ChangeState(std::unique_ptr<GameStateInterface> new_state);
  DataStore& GetDataStore() { return data_; }

 private:
  DataStore data_;
  std::unique_ptr<GameStateInterface> current_state_;
  bool running_ = true;
};

}  // namespace kernel
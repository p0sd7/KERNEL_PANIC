// game.h
#pragma once
#include <memory>
#include <stack>

#include "../DataStore/data_store.h"
#include "../RenderSystem/console_renderer.h"
#include "game_state_interface.h"

namespace kernel {

class Game {
 public:
  void run();
  void quit() { running_ = false; }
  void changeState(std::unique_ptr<GameStateInterface> new_state);
  void pushState(std::unique_ptr<GameStateInterface> new_state);
  void popState();
  DataStore& getDataStore() { return data_; }
  ConsoleRenderer& getRenderer() { return renderer_; }

 private:
  DataStore data_;
  ConsoleRenderer renderer_;
  std::stack<std::unique_ptr<GameStateInterface>> state_stack_;
  bool running_ = true;
};

}  // namespace kernel
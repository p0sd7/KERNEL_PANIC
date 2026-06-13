#pragma once

#include "game_state_interface.h"

namespace kernel {

class BaseState : public GameStateInterface {
 public:
  void setContext(Game* game) override { game_ = game; }

 protected:
  Game* game_ = nullptr;
};

}  // namespace kernel
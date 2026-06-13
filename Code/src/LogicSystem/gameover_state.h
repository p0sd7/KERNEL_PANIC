// gameover_state.h
#pragma once

#include "base_state.h"

namespace kernel {

class GameOverState : public BaseState {
 public:
  void handleInput(const InputCommand& cmd, DataStore& data,
                   ConsoleRenderer& renderer) override;
  void update(float delta, DataStore& data) override;
  void draw(const DataStore& data, ConsoleRenderer& renderer) override;
};

}  // namespace kernel
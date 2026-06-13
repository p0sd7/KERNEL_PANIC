// gameover_state.cpp
#include "gameover_state.h"

#include "../RenderSystem/console_renderer.h"
#include "game.h"

namespace kernel {

void GameOverState::handleInput(const InputCommand& cmd, DataStore& /*data*/,
                                ConsoleRenderer& /*renderer*/) {
  if (cmd.type == InputType::kConfirm || cmd.type == InputType::kQuit) {
    if (game_) game_->quit();
  }
}

void GameOverState::update(float /*delta*/, DataStore& /*data*/) {}

void GameOverState::draw(const DataStore& /*data*/, ConsoleRenderer& renderer) {
  renderer.drawGameOver();
}

}  // namespace kernel
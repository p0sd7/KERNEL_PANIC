#include "gameover_state.h"

#include "../RenderSystem/console_renderer.h"
#include "game.h"

namespace kernel {

void GameOverState::HandleInput(const InputCommand& cmd, DataStore& /*data*/) {
  if (cmd.type == InputType::kConfirm || cmd.type == InputType::kQuit) {
    if (game_) game_->Quit();
  }
}

void GameOverState::Update(float /*delta*/, DataStore& /*data*/) {}

void GameOverState::Draw(const DataStore& /*data*/) {
  RenderSystem::DrawGameOver();
}

}  // namespace kernel
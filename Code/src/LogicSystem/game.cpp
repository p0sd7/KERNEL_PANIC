#include "game.h"

#include <chrono>
#include <thread>

#include "../InputSystem/input_handler.h"
#include "../RenderSystem/console_renderer.h"
#include "exploration_state.h"

namespace kernel {

void Game::Run() {
  data_.LoadAll("../assets/");

  auto player = std::make_unique<Entity>(0, EntityType::kPlayer, '@', 10, 10);
  player->SetStat(StatType::kHp, 100);
  player->SetStat(StatType::kMaxHp, 100);
  int player_idx = data_.AddEntity(std::move(player));
  data_.SetPlayerIndex(player_idx);

  int ash_id = data_.GetLocationIdByName("/ash");
  if (ash_id == -1) {
    ash_id = 1;
  }
  data_.SetPlayerLocationId(ash_id);

  const auto& interface_config = data_.GetInterfaceConfig();
  RenderSystem::Init(interface_config);

  ChangeState(std::make_unique<ExplorationState>());

  while (running_) {
    InputCommand cmd = InputSystem::PollEvents();
    current_state_->HandleInput(cmd, data_);
    current_state_->Update(0.016f, data_);
    RenderSystem::Clear();
    current_state_->Draw(data_);
    RenderSystem::Present();
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  RenderSystem::Shutdown();
}

void Game::ChangeState(std::unique_ptr<GameStateInterface> new_state) {
  current_state_ = std::move(new_state);
  current_state_->SetContext(this);
}

}  // namespace kernel
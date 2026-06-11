#include "game.h"

#include <chrono>
#include <thread>

#include "../InputSystem/input_handler.h"
#include "../RenderSystem/console_renderer.h"
#include "exploration_state.h"

namespace kernel {

void Game::Run() {
  data_.LoadAll("../assets/");
  std::string start_loc_name = "/ash";
  int start_loc_id = data_.GetLocationIdByName(start_loc_name);
  if (start_loc_id == -1) {
    start_loc_id = 1;
  }
  std::pair<int, int> spawn = data_.GetSpawnPoint(start_loc_id);
  auto player = std::make_unique<Entity>(0, EntityType::kPlayer, '@',
                                         spawn.first, spawn.second);
  player->SetStat(StatType::kHp, 100);
  player->SetStat(StatType::kMaxHp, 100);
  int player_idx = data_.AddEntity(std::move(player));
  data_.SetPlayerIndex(player_idx);

  data_.SetPlayerLocationId(start_loc_id);

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
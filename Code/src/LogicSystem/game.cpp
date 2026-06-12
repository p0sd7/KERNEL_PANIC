#include "game.h"

#include <chrono>
#include <thread>

#include "../InputSystem/input_handler.h"
#include "../RenderSystem/console_renderer.h"
#include "exploration_state.h"

namespace kernel {

void Game::Run() {
  srand(static_cast<unsigned>(time(nullptr)));
  data_.LoadAll("../assets/");
  int start_loc_id = data_.GetLocationIdByName("/ash");
  if (start_loc_id == -1) start_loc_id = 1;
  auto spawn = data_.GetSpawnPoint(start_loc_id);
  auto player = std::make_unique<Entity>(0, EntityType::kPlayer, '@',
                                         spawn.first, spawn.second);
  player->SetStat(StatType::kHp, 100);
  player->SetStat(StatType::kMaxHp, 100);
  int player_idx = data_.AddEntity(std::move(player));
  data_.GetPlayer().entity_index = player_idx;
  data_.GetPlayer().location_id = start_loc_id;
  const auto& interface_config = data_.GetInterfaceConfig();
  RenderSystem::Init(interface_config);
  RenderSystem::SetDialogueText("Silence", {"..."});
  ChangeState(std::make_unique<ExplorationState>());

  while (running_) {
    InputCommand cmd = InputSystem::PollEvents();
    if (!state_stack_.empty()) {
      state_stack_.top()->HandleInput(cmd, data_);
      state_stack_.top()->Update(0.016f, data_);
      RenderSystem::Clear();
      state_stack_.top()->Draw(data_);
      RenderSystem::Present();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  RenderSystem::Shutdown();
}

void Game::ChangeState(std::unique_ptr<GameStateInterface> new_state) {
  while (!state_stack_.empty()) state_stack_.pop();
  if (new_state) {
    auto* base = dynamic_cast<BaseState*>(new_state.get());
    if (base) base->SetContext(this);
    state_stack_.push(std::move(new_state));
  }
}

void Game::PushState(std::unique_ptr<GameStateInterface> new_state) {
  if (new_state) {
    auto* base = dynamic_cast<BaseState*>(new_state.get());
    if (base) base->SetContext(this);
    state_stack_.push(std::move(new_state));
  }
}

void Game::PopState() {
  if (!state_stack_.empty()) state_stack_.pop();
  if (state_stack_.empty()) running_ = false;
}

}  // namespace kernel
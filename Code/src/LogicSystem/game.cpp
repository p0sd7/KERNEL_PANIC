// game.cpp
#include "game.h"

#include <chrono>
#include <thread>

#include "../InputSystem/input_handler.h"
#include "../game_constants.h"
#include "exploration_state.h"

namespace kernel {

void Game::run() {
  data_.loadAll("../assets/");
  renderer_.init(data_.getInterfaceConfig());

  int start_loc_id = data_.getLocationIdByName("/ash");
  if (start_loc_id == -1) start_loc_id = game_constants::kStartLocationId;
  auto spawn = data_.getSpawnPoint(start_loc_id);
  auto player = std::make_unique<Entity>(0, EntityType::kPlayer, '@',
                                         spawn.first, spawn.second);
  player->setStat(StatType::kHp, game_constants::kDefaultPlayerHp);
  player->setStat(StatType::kMaxHp, game_constants::kDefaultPlayerMaxHp);
  int player_idx = data_.addEntity(std::move(player));
  data_.getPlayer().entity_index = player_idx;
  data_.getPlayer().location_id = start_loc_id;

  renderer_.setDialogueText("Silence", {"..."});
  changeState(std::make_unique<ExplorationState>());

  while (running_) {
    InputCommand cmd = InputSystem::pollEvents();
    if (!state_stack_.empty()) {
      state_stack_.top()->handleInput(cmd, data_, renderer_);
      state_stack_.top()->update(0.016f, data_);
      renderer_.clear();
      state_stack_.top()->draw(data_, renderer_);
      renderer_.present();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  renderer_.shutdown();
}

void Game::changeState(std::unique_ptr<GameStateInterface> new_state) {
  while (!state_stack_.empty()) state_stack_.pop();
  if (new_state) {
    new_state->setContext(this);
    state_stack_.push(std::move(new_state));
  }
}

void Game::pushState(std::unique_ptr<GameStateInterface> new_state) {
  if (new_state) {
    new_state->setContext(this);
    state_stack_.push(std::move(new_state));
  }
}

void Game::popState() {
  if (!state_stack_.empty()) state_stack_.pop();
  if (state_stack_.empty()) running_ = false;
}

}  // namespace kernel
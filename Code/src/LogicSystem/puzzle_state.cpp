// puzzle_state.cpp
#include "puzzle_state.h"

#include "../DataStore/data_store.h"
#include "../InputSystem/input_handler.h"
#include "../Logging/logger.h"
#include "../RenderSystem/console_renderer.h"
#include "combat_state.h"
#include "game.h"

namespace kernel {

PuzzleState::PuzzleState(DataStore& data, int location_id) : solved_(false) {
  const auto* puzzle = data.getPuzzleByLocation(location_id);
  if (!puzzle) {
    logging::logError("Puzzle not found for location " +
                      std::to_string(location_id));
    solved_ = true;
    return;
  }
  solution_ = puzzle->solution_data;
  reward_item_id_ = puzzle->reward_item_id;
  wrong_penalty_ = puzzle->wrong_penalty;
  need_dialogue_set_ = true;
}

void PuzzleState::handleInput(const InputCommand& cmd, DataStore& data,
                              ConsoleRenderer& renderer) {
  if (solved_) return;
  if (cmd.type == InputType::kQuit) {
    if (game_) game_->quit();
    return;
  }
  if (cmd.type == InputType::kMove) {
    game_->popState();
    renderer.setDialogueText("Silence", {"..."});
    return;
  }
  if (cmd.type == InputType::kConfirm) {
    int row, col;
    renderer.getInputPosition(row, col);
    renderer.setCursorPosition(row + 21, col + 17);
    std::string input = InputSystem::readString();
    if (input.empty()) return;

    if (input == solution_) {
      solved_ = true;
      if (reward_item_id_ != -1) {
        const auto* item = data.getItemById(reward_item_id_);
        if (item && item->type == ItemType::kHeal) {
          Entity* player = data.getEntity(data.getPlayer().entity_index);
          if (player) {
            int new_hp =
                std::min(player->getStat(StatType::kMaxHp),
                         player->getStat(StatType::kHp) + item->effect_value);
            player->setStat(StatType::kHp, new_hp);
          }
        }
      }
      int current_loc = data.getPlayer().location_id;
      const auto* loc = data.getLocationById(current_loc);
      if (loc && loc->next_location_id != -1) {
        data.getPlayer().location_id = loc->next_location_id;
        auto spawn = data.getSpawnPoint(loc->next_location_id);
        Entity* player = data.getEntity(data.getPlayer().entity_index);
        if (player) player->setPosition(spawn.first, spawn.second);
      }
      renderer.setDialogueText("Silence", {"..."});
      game_->popState();
    } else {
      if (wrong_penalty_ == "spawn_enemies") {
        int loc_id = data.getPlayer().location_id;
        game_->pushState(std::make_unique<CombatState>(data, loc_id));
      }
    }
  }
}

void PuzzleState::update(float /*delta*/, DataStore& /*data*/) {}

void PuzzleState::draw(const DataStore& data, ConsoleRenderer& renderer) {
  if (need_dialogue_set_) {
    renderer.setDialogueText("Control panel", {"Enter the code:"});
    need_dialogue_set_ = false;
  }
  renderer.drawExploration(data, data.getPlayer().entity_index);
}

}  // namespace kernel
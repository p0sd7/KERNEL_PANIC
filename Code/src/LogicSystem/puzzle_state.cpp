#include "puzzle_state.h"

#include "../CombatSystem/combat_engine.h"
#include "../DataStore/data_store.h"
#include "../InputSystem/input_handler.h"
#include "../Logging/logger.h"
#include "../RenderSystem/console_renderer.h"
#include "combat_state.h"
#include "game.h"

namespace kernel {

PuzzleState::PuzzleState(DataStore& data, int location_id) : solved_(false) {
  const auto* puzzle = data.GetPuzzleByLocation(location_id);
  if (!puzzle) {
    logging::LogError("Puzzle not found for location " +
                      std::to_string(location_id));
    solved_ = true;
    return;
  }
  solution_ = puzzle->solution_data;
  reward_item_id_ = puzzle->reward_item_id;
  wrong_penalty_ = puzzle->wrong_penalty;
  RenderSystem::SetDialogueText("Control panel", {"Enter the code:"});
}

void PuzzleState::HandleInput(const InputCommand& cmd, DataStore& data) {
  if (solved_) return;
  if (cmd.type == InputType::kQuit) {
    if (game_) game_->Quit();
    return;
  }
  if (cmd.type == InputType::kMove) {
    game_->PopState();
    RenderSystem::SetDialogueText("Silence", {"..."});
    return;
  }
  if (cmd.type == InputType::kConfirm) {
    int x, y;
    RenderSystem::GetInputPosition(x, y);
    RenderSystem::SetCursorPosition(x, y + 13);
    std::string input = InputSystem::ReadString();
    if (input.empty()) return;

    if (input == solution_) {
      solved_ = true;
      if (reward_item_id_ != -1) {
        const auto* item = data.GetItemById(reward_item_id_);
        if (item && item->type == ItemType::kHeal) {
          Entity* player = data.GetEntity(data.GetPlayer().entity_index);
          if (player) {
            int new_hp =
                std::min(player->GetStat(StatType::kMaxHp),
                         player->GetStat(StatType::kHp) + item->effect_value);
            player->SetStat(StatType::kHp, new_hp);
          }
        }
      }
      int current_loc = data.GetPlayer().location_id;
      const auto* loc = data.GetLocationById(current_loc);
      if (loc && loc->next_location_id != -1) {
        data.GetPlayer().location_id = loc->next_location_id;
        auto spawn = data.GetSpawnPoint(loc->next_location_id);
        Entity* player = data.GetEntity(data.GetPlayer().entity_index);
        if (player) player->SetPosition(spawn.first, spawn.second);
      }
      RenderSystem::SetDialogueText("Silence", {"..."});
      game_->PopState();
      return;
    } else {
      if (wrong_penalty_ == "spawn_enemies") {
        int loc_id = data.GetPlayer().location_id;
        const auto& group_entries = data.GetEnemyGroup(loc_id);
        game_->PushState(std::make_unique<CombatState>(data, loc_id));
      }
    }
  }
}

void PuzzleState::Update(float /*delta*/, DataStore& /*data*/) {}

void PuzzleState::Draw(const DataStore& data) {
  int player_idx = data.GetPlayer().entity_index;
  RenderSystem::DrawExploration(data, player_idx);
}

}  // namespace kernel
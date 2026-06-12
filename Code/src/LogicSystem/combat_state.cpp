#include "combat_state.h"

#include <chrono>
#include <thread>

#include "../CombatSystem/combat_engine.h"
#include "../DataStore/data_store.h"
#include "../InputSystem/input_handler.h"
#include "../RenderSystem/console_renderer.h"
#include "game.h"
#include "gameover_state.h"

namespace kernel {

CombatState::CombatState(DataStore& data, const std::vector<int>& enemy_indices,
                         bool is_boss_fight)
    : enemy_indices_(enemy_indices),
      is_boss_fight_(is_boss_fight),
      last_script_used_id_(-1),
      player_defense_percent_(0),
      combat_over_(false),
      highlight_enemy_(-1) {
  CombatEngine::StartCombat(data, data.GetPlayer().entity_index,
                            enemy_indices_);
  combat_log_ = "";
}

void CombatState::HandleInput(const InputCommand& cmd, DataStore& data) {
  if (combat_over_) return;
  if (cmd.type == InputType::kQuit) {
    if (game_) game_->Quit();
    return;
  }
  if (cmd.type == InputType::kConfirm) {
    RenderSystem::FlushInput();
    int y, x;
    RenderSystem::GetInputPosition(y, x);
    RenderSystem::SetCursorPosition(y, x);
    std::string input = InputSystem::ReadString();
    if (input.empty()) return;

    std::string script_name = input;
    int target_num = 1;
    size_t space_pos = input.find(' ');
    if (space_pos != std::string::npos) {
      script_name = input.substr(0, space_pos);
      std::string num_str = input.substr(space_pos + 1);
      try {
        target_num = std::stoi(num_str);
        if (target_num < 1 ||
            target_num > static_cast<int>(enemy_indices_.size())) {
          combat_log_ = "Invalid target number.\n";
          return;
        }
      } catch (...) {
        combat_log_ = "Invalid target number.\n";
        return;
      }
    }

    int script_id = data.GetScriptIdByName(script_name);
    if (script_id == -1) {
      combat_log_ = "Unknown script.\n";
      return;
    }
    if (!data.HasScriptInInventory(script_id)) {
      combat_log_ = "You don't have this script.\n";
      return;
    }

    int target_index = target_num - 1;
    std::string log;
    bool success = CombatEngine::ApplyScript(
        data, data.GetPlayer().entity_index, script_id, enemy_indices_,
        target_index, player_defense_percent_, log);
    if (!success) {
      combat_log_ = "Failed to apply script.\n";
      return;
    }

    highlight_enemy_ = target_index;
    RenderSystem::DrawCombat(data, data.GetPlayer().entity_index,
                             enemy_indices_, combat_log_, highlight_enemy_);
    RenderSystem::Present();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    highlight_enemy_ = -1;

    if (CombatEngine::IsCombatOver(enemy_indices_, data)) {
      combat_over_ = true;
      // потом награда за победу
      for (int idx : enemy_indices_) data.RemoveEntity(idx);
      game_->PopState();
      return;
    }

    std::string enemy_log;
    CombatEngine::EnemyTurn(data, data.GetPlayer().entity_index, enemy_indices_,
                            player_defense_percent_, enemy_log);
    player_defense_percent_ = 0;

    Entity* player = data.GetEntity(data.GetPlayer().entity_index);
    if (player && player->GetStat(StatType::kHp) <= 0) {
      combat_over_ = true;
      game_->ChangeState(std::make_unique<GameOverState>());
      return;
    }

    if (CombatEngine::IsCombatOver(enemy_indices_, data)) {
      combat_over_ = true;
      for (int idx : enemy_indices_) data.RemoveEntity(idx);
      game_->PopState();
    }
    combat_log_ = "";
  }
}

void CombatState::Update(float /*delta*/, DataStore& /*data*/) {}

void CombatState::Draw(const DataStore& data) {
  int player_idx = data.GetPlayer().entity_index;
  RenderSystem::DrawCombat(data, player_idx, enemy_indices_, combat_log_,
                           highlight_enemy_);
}

}  // namespace kernel
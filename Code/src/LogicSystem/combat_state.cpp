#include "combat_state.h"

#include <chrono>
#include <cstdlib>
#include <thread>

#include "../CombatSystem/combat_engine.h"
#include "../DataStore/data_store.h"
#include "../InputSystem/input_handler.h"
#include "../RenderSystem/console_renderer.h"
#include "game.h"
#include "gameover_state.h"

namespace kernel {

CombatState::CombatState(DataStore& data, const std::vector<int>& enemy_indices,
                         bool is_boss_fight, int location_after_boss)
    : enemy_indices_(enemy_indices),
      is_boss_fight_(is_boss_fight),
      location_after_boss_(location_after_boss),
      last_script_used_id_(-1),
      player_defense_percent_(0),
      combat_over_(false),
      highlight_enemy_(-1),
      boss_id_(-1),
      spawn_dialogue_shown_(false) {
  CombatEngine::StartCombat(data, data.GetPlayer().entity_index, enemy_indices_,
                            boss_id_, boss_name_, spawn_dialogue_shown_);
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
      try {
        target_num = std::stoi(input.substr(space_pos + 1));
        if (target_num < 1 ||
            target_num > static_cast<int>(enemy_indices_.size())) {
          goto enemy_turn;
        }
      } catch (...) {
        goto enemy_turn;
      }
    }
    {
      int script_id = data.GetScriptIdByName(script_name);
      if (script_id == -1) {
        goto enemy_turn;
      }

      int target_index = target_num - 1;
      std::string log;
      bool success = CombatEngine::ApplyScript(
          data, data.GetPlayer().entity_index, script_id, enemy_indices_,
          target_index, player_defense_percent_, log);
      if (!success) {
        goto enemy_turn;
      }
      highlight_enemy_ = target_index;
      RenderSystem::DrawCombat(data, data.GetPlayer().entity_index,
                               enemy_indices_, "", highlight_enemy_,
                               is_boss_fight_, boss_id_);
      RenderSystem::Present();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      highlight_enemy_ = -1;

      if (CombatEngine::IsCombatOver(enemy_indices_, data)) {
        combat_over_ = true;
        if (is_boss_fight_) {
          data.SetMemoryPercent(data.GetMemoryPercent() + 30);
          for (const auto& [id, scr] : data.GetScripts()) {
            if (scr.available_after_boss == std::to_string(boss_id_) &&
                !data.HasScriptInInventory(scr.id)) {
              data.AddScriptToInventory(scr.id);
            }
          }
          if (location_after_boss_ != -1) {
            data.GetPlayer().location_id = location_after_boss_;
            auto spawn = data.GetSpawnPoint(location_after_boss_);
            Entity* player = data.GetEntity(data.GetPlayer().entity_index);
            if (player) player->SetPosition(spawn.first, spawn.second);
          }
          RenderSystem::SetCombatDialogue("", {});
        } else {
          int heal = 10;
          Entity* player = data.GetEntity(data.GetPlayer().entity_index);
          if (player) {
            int new_hp = std::min(player->GetStat(StatType::kMaxHp),
                                  player->GetStat(StatType::kHp) + heal);
            player->SetStat(StatType::kHp, new_hp);
          }
          RenderSystem::SetCombatDialogue("", {});
        }
        for (int idx : enemy_indices_) data.RemoveEntity(idx);
        game_->PopState();
        return;
      }
    }
    {
      std::string enemy_log;
      CombatEngine::EnemyTurn(data, data.GetPlayer().entity_index,
                              enemy_indices_, player_defense_percent_,
                              enemy_log);
      player_defense_percent_ = 0;
      if (!enemy_log.empty()) {
        RenderSystem::SetCombatDialogue("Enemy", {enemy_log});
      }

      Entity* player = data.GetEntity(data.GetPlayer().entity_index);
      if (player && player->GetStat(StatType::kHp) <= 0) {
        combat_over_ = true;
        game_->ChangeState(std::make_unique<GameOverState>());
        return;
      }

      if (CombatEngine::IsCombatOver(enemy_indices_, data)) {
        combat_over_ = true;
        if (is_boss_fight_) {
          data.SetMemoryPercent(data.GetMemoryPercent() + 30);
          for (const auto& [id, scr] : data.GetScripts()) {
            if (scr.available_after_boss == std::to_string(boss_id_) &&
                !data.HasScriptInInventory(scr.id)) {
              data.AddScriptToInventory(scr.id);
            }
          }
          if (location_after_boss_ != -1) {
            data.GetPlayer().location_id = location_after_boss_;
            auto spawn = data.GetSpawnPoint(location_after_boss_);
            Entity* player = data.GetEntity(data.GetPlayer().entity_index);
            if (player) player->SetPosition(spawn.first, spawn.second);
          }
          int heal = CombatEngine::GetHealReward(enemy_indices_, data);
          if (player) {
            int new_hp = std::min(player->GetStat(StatType::kMaxHp),
                                  player->GetStat(StatType::kHp) + heal);
            player->SetStat(StatType::kHp, new_hp);
          }
          RenderSystem::SetCombatDialogue("", {});
        } else {
          RenderSystem::SetCombatDialogue("", {});
        }
        for (int idx : enemy_indices_) data.RemoveEntity(idx);
        game_->PopState();
        return;
      }
    }
    return;

  enemy_turn: {
    std::string enemy_log;
    CombatEngine::EnemyTurn(data, data.GetPlayer().entity_index, enemy_indices_,
                            player_defense_percent_, enemy_log);
    player_defense_percent_ = 0;
    if (!enemy_log.empty()) {
      RenderSystem::SetCombatDialogue("Enemy", {enemy_log});
    }

    Entity* player = data.GetEntity(data.GetPlayer().entity_index);
    if (player && player->GetStat(StatType::kHp) <= 0) {
      combat_over_ = true;
      game_->ChangeState(std::make_unique<GameOverState>());
      return;
    }

    if (CombatEngine::IsCombatOver(enemy_indices_, data)) {
      combat_over_ = true;
      int heal = CombatEngine::GetHealReward(enemy_indices_, data);
      Entity* player = data.GetEntity(data.GetPlayer().entity_index);
      if (player) {
        int new_hp = std::min(player->GetStat(StatType::kMaxHp),
                              player->GetStat(StatType::kHp) + heal);
        player->SetStat(StatType::kHp, new_hp);
      }
      for (int idx : enemy_indices_) data.RemoveEntity(idx);
      game_->PopState();
    }
  }
  }
}

void CombatState::Update(float /*delta*/, DataStore& /*data*/) {}

void CombatState::Draw(const DataStore& data) {
  int player_idx = data.GetPlayer().entity_index;
  RenderSystem::DrawCombat(data, player_idx, enemy_indices_, combat_log_,
                           highlight_enemy_, is_boss_fight_, boss_id_);
}

}  // namespace kernel
#include "combat_state.h"

#include <chrono>
#include <cstdlib>
#include <thread>

#include "../CombatSystem/combat_engine.h"
#include "../DataStore/data_store.h"
#include "../InputSystem/input_handler.h"
#include "../Logging/logger.h"
#include "../RenderSystem/console_renderer.h"
#include "final_state.h"
#include "game.h"
#include "gameover_state.h"

namespace kernel {

CombatState::CombatState(DataStore& data, int enemy_id, char symbol)
    : is_boss_fight_(true),
      last_script_used_id_(-1),
      player_defense_percent_(0),
      combat_over_(false),
      highlight_enemy_(-1),
      boss_id_(-1),
      spawn_dialogue_shown_(false) {
  const auto* templ = data.GetEnemyTemplate(enemy_id);
  if (!templ) {
    logging::LogError("Unknown enemy id: " + std::to_string(enemy_id));
    combat_over_ = true;
    return;
  }

  auto enemy = std::make_unique<Entity>(templ->id, EntityType::kEnemy, symbol);
  enemy->SetStat(StatType::kHp, templ->hp);
  enemy->SetStat(StatType::kMaxHp, templ->hp);
  enemy->SetStat(StatType::kDamage, templ->damage);
  enemy->SetStat(StatType::kEnemyId, templ->id);
  int idx = data.AddEntity(std::move(enemy));
  enemy_indices_.push_back(idx);

  boss_name_ = templ->name;
  boss_id_ = enemy_id;
  if (!templ->dialogue_on_spawn.empty()) {
    RenderSystem::SetCombatDialogue(boss_name_, {templ->dialogue_on_spawn});
    spawn_dialogue_shown_ = true;
  }
  combat_log_ = "";
}

CombatState::CombatState(DataStore& data, int location_id)
    : is_boss_fight_(false),
      last_script_used_id_(-1),
      player_defense_percent_(0),
      combat_over_(false),
      highlight_enemy_(-1),
      boss_id_(-1),
      spawn_dialogue_shown_(false) {
  const auto& group_entries = data.GetEnemyGroup(location_id);
  if (group_entries.empty()) {
    logging::LogError("No enemies for group at location " +
                      std::to_string(location_id));
    combat_over_ = true;
    return;
  }
  for (const auto& entry : group_entries) {
    int count = entry.min_count;
    if (entry.max_count > entry.min_count) {
      count += rand() % (entry.max_count - entry.min_count + 1);
    }
    for (int i = 0; i < count; ++i) {
      const auto* templ = data.GetEnemyTemplate(entry.enemy_id);
      if (!templ) continue;
      auto enemy = std::make_unique<Entity>(templ->id, EntityType::kEnemy, '?');
      enemy->SetStat(StatType::kHp, templ->hp);
      enemy->SetStat(StatType::kMaxHp, templ->hp);
      enemy->SetStat(StatType::kDamage, templ->damage);
      enemy->SetStat(StatType::kEnemyId, templ->id);
      enemy_indices_.push_back(data.AddEntity(std::move(enemy)));
    }
  }
  if (!enemy_indices_.empty()) {
    int first_id =
        data.GetEntity(enemy_indices_[0])->GetStat(StatType::kEnemyId);
    const auto* templ = data.GetEnemyTemplate(first_id);
    if (templ && !templ->dialogue_on_spawn.empty()) {
      RenderSystem::SetCombatDialogue(templ->name, {templ->dialogue_on_spawn});
      spawn_dialogue_shown_ = true;
    }
  }
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
          RenderSystem::SetCombatDialogue("", {});
        } else {
          int heal = CombatEngine::GetHealReward(enemy_indices_, data);
          Entity* player = data.GetEntity(data.GetPlayer().entity_index);
          if (player) {
            int new_hp = std::min(player->GetStat(StatType::kMaxHp),
                                  player->GetStat(StatType::kHp) + heal);
            player->SetStat(StatType::kHp, new_hp);
          }
          RenderSystem::SetCombatDialogue("", {});
        }
        for (int idx : enemy_indices_) data.RemoveEntity(idx);
        if (data.GetPlayer().location_id != 8) {
          game_->PopState();
        } else {
          game_->ChangeState(std::make_unique<FinalState>());
        }
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
        if (data.GetPlayer().location_id != 8) {
          game_->PopState();
        } else {
          game_->ChangeState(std::make_unique<FinalState>());
        }
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
      if (data.GetPlayer().location_id != 8) {
        game_->PopState();
      } else {
        game_->ChangeState(std::make_unique<FinalState>());
      }
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
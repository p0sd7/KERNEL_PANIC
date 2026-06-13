#include "combat_engine.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>

#include "../DataStore/data_store.h"
#include "../Entities/entity.h"
#include "../RenderSystem/console_renderer.h"

namespace kernel {
namespace CombatEngine {

static std::vector<int> stun_counters_;

static int GetRandom(int min, int max) {
  static bool seeded = false;
  if (!seeded) {
    srand(static_cast<unsigned>(time(nullptr)));
    seeded = true;
  }
  return min + rand() % (max - min + 1);
}

void StartCombat(DataStore& data, int player_idx,
                 std::vector<int>& enemy_indices, int& boss_id,
                 std::string& boss_name, bool& spawn_dialogue_shown) {
  (void)player_idx;
  boss_id = -1;
  boss_name.clear();
  spawn_dialogue_shown = false;

  stun_counters_.resize(enemy_indices.size(), 0);

  if (!enemy_indices.empty()) {
    int enemy_id =
        data.GetEntity(enemy_indices[0])->GetStat(StatType::kEnemyId);
    const auto* templ = data.GetEnemyTemplate(enemy_id);
    if (templ) {
      boss_name = templ->name;
      if (templ->enemy_type == EnemyType::kBoss) {
        boss_id = enemy_id;
      }
      if (!templ->dialogue_on_spawn.empty()) {
        RenderSystem::SetCombatDialogue(templ->name,
                                        {templ->dialogue_on_spawn});
        spawn_dialogue_shown = true;
      }
    }
  }
}

bool ApplyScript(DataStore& data, int player_idx, int script_id,
                 std::vector<int>& enemy_indices, int target_idx,
                 int& player_defense_percent, std::string& log) {
  const auto* script = data.GetScriptById(script_id);
  if (!script) {
    return false;
  }

  Entity* player = data.GetEntity(player_idx);
  if (!player) return false;

  if (target_idx < 0 || target_idx >= static_cast<int>(enemy_indices.size())) {
    return false;
  }
  int enemy_index = enemy_indices[target_idx];
  Entity* enemy = data.GetEntity(enemy_index);
  if (!enemy || enemy->GetStat(StatType::kHp) <= 0) {
    return false;
  }

  if (script->damage > 0) {
    int new_hp = enemy->GetStat(StatType::kHp) - script->damage;
    enemy->SetStat(StatType::kHp, std::max(0, new_hp));
  }

  if (script->self_damage > 0) {
    int new_hp = player->GetStat(StatType::kHp) - script->self_damage;
    player->SetStat(StatType::kHp, std::max(0, new_hp));
  }

  if (script->defense_percent > 0) {
    player_defense_percent = script->defense_percent;
  }

  if (script->stun_target) {
    // target_idx — порядковый номер врага (0..size-1)
    if (target_idx >= 0 &&
        target_idx < static_cast<int>(stun_counters_.size())) {
      stun_counters_[target_idx] = 2;  // оглушение на 2 хода
    }
  }

  return true;
}

void EnemyTurn(DataStore& data, int player_idx, std::vector<int>& enemy_indices,
               int player_defense_percent, std::string& log) {
  Entity* player = data.GetEntity(player_idx);
  if (!player) return;
  log.clear();

  for (size_t i = 0; i < enemy_indices.size(); ++i) {
    // Проверяем оглушение для врага с порядковым номером i
    if (static_cast<int>(i) < static_cast<int>(stun_counters_.size()) &&
        stun_counters_[i] > 0) {
      stun_counters_[i]--;
      continue;
    }

    Entity* enemy = data.GetEntity(enemy_indices[i]);
    if (!enemy || enemy->GetStat(StatType::kHp) <= 0) continue;

    int damage = enemy->GetStat(StatType::kDamage);
    if (player_defense_percent > 0) {
      damage = damage * (100 - player_defense_percent) / 100;
    }
    int new_hp = player->GetStat(StatType::kHp) - damage;
    player->SetStat(StatType::kHp, std::max(0, new_hp));

    // Можно добавить вывод в лог
    // int enemy_id = enemy->GetStat(StatType::kEnemyId);
    // const auto* templ = data.GetEnemyTemplate(enemy_id);
    // log += templ ? templ->name : "Enemy";
    // log += " attacks you for " + std::to_string(damage) + " damage.\n";
  }
}

bool IsCombatOver(const std::vector<int>& enemy_indices,
                  const DataStore& data) {
  for (int idx : enemy_indices) {
    const Entity* enemy = data.GetEntity(idx);
    if (enemy && enemy->GetStat(StatType::kHp) > 0) return false;
  }
  stun_counters_.clear();
  return true;
}

int GetTotalEnemyHp(const std::vector<int>& enemy_indices,
                    const DataStore& data) {
  int total = 0;
  for (int idx : enemy_indices) {
    const Entity* enemy = data.GetEntity(idx);
    if (enemy) total += enemy->GetStat(StatType::kHp);
  }
  return total;
}

int GetHealReward(const std::vector<int>& enemy_indices,
                  const DataStore& data) {
  for (int idx : enemy_indices) {
    const Entity* enemy = data.GetEntity(idx);
    if (!enemy) continue;
    int enemy_id = enemy->GetStat(StatType::kEnemyId);
    const auto* templ = data.GetEnemyTemplate(enemy_id);
    if (templ && templ->enemy_type == EnemyType::kBoss) {
      return 30;
    }
  }
  return 5;
}

}  // namespace CombatEngine
}  // namespace kernel
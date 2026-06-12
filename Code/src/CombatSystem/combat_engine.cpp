#include "combat_engine.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>

#include "../DataStore/data_store.h"
#include "../Entities/entity.h"

namespace kernel {
namespace CombatEngine {

static int GetRandom(int min, int max) {
  static bool seeded = false;
  if (!seeded) {
    srand(static_cast<unsigned>(time(nullptr)));
    seeded = true;
  }
  return min + rand() % (max - min + 1);
}

void StartCombat(DataStore& data, int player_idx,
                 std::vector<int>& enemy_indices) {
  (void)data;
  (void)player_idx;
  (void)enemy_indices;
}

bool ApplyScript(DataStore& data, int player_idx, int script_id,
                 std::vector<int>& enemy_indices, int target_idx,
                 int& player_defense_percent, std::string& log) {
  const auto* script = data.GetScriptById(script_id);
  if (!script) {
    log = "Script not found.\n";
    return false;
  }

  Entity* player = data.GetEntity(player_idx);
  if (!player) return false;

  if (target_idx < 0 || target_idx >= static_cast<int>(enemy_indices.size())) {
    log = "Invalid target index.\n";
    return false;
  }
  int enemy_index = enemy_indices[target_idx];
  Entity* enemy = data.GetEntity(enemy_index);
  if (!enemy || enemy->GetStat(StatType::kHp) <= 0) {
    log = "Target is already dead.\n";
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
    // сделать оглушение
  }

  return true;
}

void EnemyTurn(DataStore& data, int player_idx, std::vector<int>& enemy_indices,
               int player_defense_percent, std::string& log) {
  Entity* player = data.GetEntity(player_idx);
  if (!player) return;

  for (int idx : enemy_indices) {
    Entity* enemy = data.GetEntity(idx);
    if (!enemy || enemy->GetStat(StatType::kHp) <= 0) continue;

    int damage = enemy->GetStat(StatType::kDamage);
    if (player_defense_percent > 0) {
      damage = damage * (100 - player_defense_percent) / 100;
    }
    int new_hp = player->GetStat(StatType::kHp) - damage;
    player->SetStat(StatType::kHp, std::max(0, new_hp));
  }
}

bool IsCombatOver(const std::vector<int>& enemy_indices,
                  const DataStore& data) {
  for (int idx : enemy_indices) {
    const Entity* enemy = data.GetEntity(idx);
    if (enemy && enemy->GetStat(StatType::kHp) > 0) return false;
  }
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
  (void)enemy_indices;
  (void)data;
  return 0;
}

}  // namespace CombatEngine
}  // namespace kernel
#include "combat_engine.h"

#include <algorithm>

#include "../DataStore/data_store.h"
#include "../Entities/entity.h"
#include "../Random/random_utils.h"
#include "../game_constants.h"

namespace kernel {

void CombatEngine::startCombat(DataStore& data, int /*player_idx*/,
                               std::vector<int>& enemy_indices, int& boss_id,
                               std::string& boss_name,
                               bool& spawn_dialogue_shown) {
  boss_id = -1;
  boss_name.clear();
  spawn_dialogue_shown = false;

  stun_counters_.resize(enemy_indices.size(), 0);

  if (!enemy_indices.empty()) {
    Entity* first_enemy = data.getEntity(enemy_indices[0]);
    if (first_enemy) {
      int enemy_id = first_enemy->getStat(StatType::kEnemyId);
      const auto* templ = data.getEnemyTemplate(enemy_id);
      if (templ) {
        boss_name = templ->name;
        if (templ->enemy_type == EnemyType::kBoss) {
          boss_id = enemy_id;
        }
        if (!templ->dialogue_on_spawn.empty()) {
          spawn_dialogue_shown = true;
        }
      }
    }
  }
}

bool CombatEngine::applyScript(DataStore& data, int player_idx, int script_id,
                               std::vector<int>& enemy_indices, int target_idx,
                               int& player_defense_percent, std::string& log) {
  const auto* script = data.getScriptById(script_id);
  if (!script) {
    return false;
  }

  Entity* player = data.getEntity(player_idx);
  if (!player) return false;

  if (target_idx < 0 || target_idx >= static_cast<int>(enemy_indices.size())) {
    return false;
  }
  int enemy_index = enemy_indices[target_idx];
  Entity* enemy = data.getEntity(enemy_index);
  if (!enemy || enemy->getStat(StatType::kHp) <= 0) {
    return false;
  }

  if (script->damage > 0) {
    int new_hp = enemy->getStat(StatType::kHp) - script->damage;
    enemy->setStat(StatType::kHp, std::max(0, new_hp));
  }

  if (script->self_damage > 0) {
    int new_hp = player->getStat(StatType::kHp) - script->self_damage;
    player->setStat(StatType::kHp, std::max(0, new_hp));
  }

  if (script->defense_percent > 0) {
    player_defense_percent = script->defense_percent;
  }

  if (script->stun_target) {
    if (target_idx >= 0 &&
        target_idx < static_cast<int>(stun_counters_.size())) {
      stun_counters_[target_idx] = game_constants::kStunTurns;
    }
  }

  return true;
}

void CombatEngine::enemyTurn(DataStore& data, int player_idx,
                             std::vector<int>& enemy_indices,
                             int player_defense_percent, std::string& log) {
  Entity* player = data.getEntity(player_idx);
  if (!player) return;
  log.clear();

  for (size_t i = 0; i < enemy_indices.size(); ++i) {
    if (static_cast<int>(i) < static_cast<int>(stun_counters_.size()) &&
        stun_counters_[i] > 0) {
      --stun_counters_[i];
      continue;
    }

    Entity* enemy = data.getEntity(enemy_indices[i]);
    if (!enemy || enemy->getStat(StatType::kHp) <= 0) continue;

    int damage = enemy->getStat(StatType::kDamage);
    if (player_defense_percent > 0) {
      damage = damage * (100 - player_defense_percent) / 100;
    }
    int new_hp = player->getStat(StatType::kHp) - damage;
    player->setStat(StatType::kHp, std::max(0, new_hp));
  }
}

bool CombatEngine::isCombatOver(const std::vector<int>& enemy_indices,
                                const DataStore& data) {
  for (int idx : enemy_indices) {
    const Entity* enemy = data.getEntity(idx);
    if (enemy && enemy->getStat(StatType::kHp) > 0) return false;
  }
  stun_counters_.clear();
  return true;
}

int CombatEngine::getTotalEnemyHp(const std::vector<int>& enemy_indices,
                                  const DataStore& data) {
  int total = 0;
  for (int idx : enemy_indices) {
    const Entity* enemy = data.getEntity(idx);
    if (enemy) total += enemy->getStat(StatType::kHp);
  }
  return total;
}

int CombatEngine::getHealReward(const std::vector<int>& enemy_indices,
                                const DataStore& data) {
  for (int idx : enemy_indices) {
    const Entity* enemy = data.getEntity(idx);
    if (!enemy) continue;
    int enemy_id = enemy->getStat(StatType::kEnemyId);
    const auto* templ = data.getEnemyTemplate(enemy_id);
    if (templ && templ->enemy_type == EnemyType::kBoss) {
      return 30;
    }
  }
  return 5;
}

}  // namespace kernel
#pragma once

#include <string>
#include <vector>

#include "../DataStore/data_store.h"

namespace kernel {
namespace CombatEngine {

void StartCombat(DataStore& data, int player_idx,
                 std::vector<int>& enemy_indices, int& boss_id,
                 std::string& boss_name, bool& spawn_dialogue_shown);

bool ApplyScript(DataStore& data, int player_idx, int script_id,
                 std::vector<int>& enemy_indices, int target_idx,
                 int& player_defense_percent, std::string& log);

void EnemyTurn(DataStore& data, int player_idx, std::vector<int>& enemy_indices,
               int player_defense_percent, std::string& log);

bool IsCombatOver(const std::vector<int>& enemy_indices, const DataStore& data);

int GetTotalEnemyHp(const std::vector<int>& enemy_indices,
                    const DataStore& data);

int GetHealReward(const std::vector<int>& enemy_indices, const DataStore& data);

}  // namespace CombatEngine
}  // namespace kernel
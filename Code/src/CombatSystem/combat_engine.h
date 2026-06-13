#pragma once

#include <string>
#include <vector>

#include "../DataStore/data_store.h"

namespace kernel {

class CombatEngine {
 public:
  void startCombat(DataStore& data, int player_idx,
                   std::vector<int>& enemy_indices, int& boss_id,
                   std::string& boss_name, bool& spawn_dialogue_shown);

  bool applyScript(DataStore& data, int player_idx, int script_id,
                   std::vector<int>& enemy_indices, int target_idx,
                   int& player_defense_percent, std::string& log);

  void enemyTurn(DataStore& data, int player_idx,
                 std::vector<int>& enemy_indices, int player_defense_percent,
                 std::string& log);

  bool isCombatOver(const std::vector<int>& enemy_indices,
                    const DataStore& data);

  int getTotalEnemyHp(const std::vector<int>& enemy_indices,
                      const DataStore& data);
  int getHealReward(const std::vector<int>& enemy_indices,
                    const DataStore& data);

 private:
  std::vector<int> stun_counters_;
};

}  // namespace kernel
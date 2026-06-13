#pragma once

#include <string>
#include <vector>

#include "../CombatSystem/combat_engine.h"
#include "base_state.h"

namespace kernel {

class CombatState : public BaseState {
 public:
  CombatState(DataStore& data, int enemy_id, char symbol);
  CombatState(DataStore& data, int location_id);

  void handleInput(const InputCommand& cmd, DataStore& data,
                   ConsoleRenderer& renderer) override;
  void update(float delta, DataStore& data) override;
  void draw(const DataStore& data, ConsoleRenderer& renderer) override;

 private:
  void applyTurnResult(DataStore& data);

  std::vector<int> enemy_indices_;
  bool is_boss_fight_ = false;
  int player_defense_percent_ = 0;
  bool combat_over_ = false;
  int highlight_enemy_ = -1;
  int boss_id_ = -1;
  std::string boss_name_;
  std::string combat_log_;
  bool spawn_dialogue_shown_ = false;
  CombatEngine combat_engine_;
};

}  // namespace kernel
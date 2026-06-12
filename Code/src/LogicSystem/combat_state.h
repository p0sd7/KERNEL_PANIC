#pragma once

#include <string>
#include <vector>

#include "base_state.h"

namespace kernel {

class CombatState : public BaseState {
 public:
  CombatState(DataStore& data, int enemy_id, char symbol);

  CombatState(DataStore& data, int location_id);

  void HandleInput(const InputCommand& cmd, DataStore& data) override;
  void Update(float delta, DataStore& data) override;
  void Draw(const DataStore& data) override;

 private:
  std::vector<int> enemy_indices_;
  bool is_boss_fight_;
  int last_script_used_id_;
  int player_defense_percent_;
  bool combat_over_;
  int highlight_enemy_;
  int boss_id_;
  std::string boss_name_;
  std::string combat_log_;
  bool spawn_dialogue_shown_;
};

}  // namespace kernel
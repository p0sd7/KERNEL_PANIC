// puzzle_state.h
#pragma once

#include <string>

#include "base_state.h"

namespace kernel {

class PuzzleState : public BaseState {
 public:
  PuzzleState(DataStore& data, int location_id);
  void handleInput(const InputCommand& cmd, DataStore& data,
                   ConsoleRenderer& renderer) override;
  void update(float delta, DataStore& data) override;
  void draw(const DataStore& data, ConsoleRenderer& renderer) override;

 private:
  std::string solution_;
  int reward_item_id_ = -1;
  std::string wrong_penalty_;
  bool solved_ = false;
  bool need_dialogue_set_ = false;
};

}  // namespace kernel
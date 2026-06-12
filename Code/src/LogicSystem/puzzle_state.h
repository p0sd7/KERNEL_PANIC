#pragma once

#include <string>

#include "base_state.h"

namespace kernel {

class PuzzleState : public BaseState {
 public:
  PuzzleState(DataStore& data, int location_id);
  void HandleInput(const InputCommand& cmd, DataStore& data) override;
  void Update(float delta, DataStore& data) override;
  void Draw(const DataStore& data) override;

 private:
  std::string solution_;
  int reward_item_id_;
  std::string wrong_penalty_;
  bool solved_;
};

}  // namespace kernel
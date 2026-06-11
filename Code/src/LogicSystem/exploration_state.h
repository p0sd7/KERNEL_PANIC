#pragma once

#include "base_state.h"

namespace kernel {

class ExplorationState : public BaseState {
 public:
  ExplorationState();
  void HandleInput(const InputCommand& cmd, DataStore& data) override;
  void Update(float delta, DataStore& data) override;
  void Draw(const DataStore& data) override;

 private:
  bool show_inventory_ = false;
};

}  // namespace kernel
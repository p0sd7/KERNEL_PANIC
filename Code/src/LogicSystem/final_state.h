#pragma once

#include "base_state.h"

namespace kernel {

class FinalState : public BaseState {
 public:
  FinalState();
  void handleInput(const InputCommand& cmd, DataStore& data,
                   ConsoleRenderer& renderer) override;
  void update(float delta, DataStore& data) override;
  void draw(const DataStore& data, ConsoleRenderer& renderer) override;

 private:
  enum class Step {
    kWaitStart,
    kLine1,
    kPause1,
    kLine2,
    kPause2,
    kLine3,
    kPause3,
    kPrompt
  };
  Step step_ = Step::kWaitStart;
  float timer_ = 0.0f;
  bool input_active_ = false;
};

}  // namespace kernel
// dialogue_state.h
#pragma once

#include <string>
#include <vector>

#include "../DataStore/data_store.h"
#include "base_state.h"

namespace kernel {

class DialogueState : public BaseState {
 public:
  DialogueState(DataStore& data, int npc_id);
  void handleInput(const InputCommand& cmd, DataStore& data,
                   ConsoleRenderer& renderer) override;
  void update(float delta, DataStore& data) override;
  void draw(const DataStore& data, ConsoleRenderer& renderer) override;

 private:
  std::string npc_name_;
  std::vector<DialogueLine> lines_;
  size_t current_line_index_ = 0;
  bool finished_ = false;
  bool first_frame_ = true;
};

}  // namespace kernel
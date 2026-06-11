#pragma once

#include <string>
#include <vector>

#include "../DataStore/data_store.h"
#include "base_state.h"

namespace kernel {

class DialogueState : public BaseState {
 public:
  DialogueState(DataStore& data, int npc_id);
  void HandleInput(const InputCommand& cmd, DataStore& data) override;
  void Update(float delta, DataStore& data) override;
  void Draw(const DataStore& data) override;

 private:
  std::string npc_name_;
  std::vector<DialogueLine> lines_;
  size_t current_line_index_ = 0;
  bool finished_ = false;
  bool first_frame_ = true;
};

}  // namespace kernel
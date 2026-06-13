#include "dialogue_state.h"

#include "../DataStore/data_store.h"
#include "../Logging/logger.h"
#include "../RenderSystem/console_renderer.h"
#include "game.h"

namespace kernel {

DialogueState::DialogueState(DataStore& data, int npc_id) {
  npc_name_ = data.GetNpcName(npc_id);
  lines_ = data.GetDialoguesForNpc(npc_id, data.GetMemoryPercent(),
                                   data.GetFragments());
  if (!lines_.empty()) {
    if (npc_name_ == "healer.dll") {
      int new_mem = data.GetPlayer().memory_percent + 5;

      if (!data.HasScriptInInventory(6)) {
        data.AddScriptToInventory(6);
        data.SetMemoryPercent(new_mem);
      }
    }
    RenderSystem::SetDialogueText(npc_name_, {lines_[0].text});
  } else {
    RenderSystem::SetDialogueText(npc_name_, {"..."});
  }
}

void DialogueState::HandleInput(const InputCommand& cmd, DataStore& /*data*/) {
  if (finished_) return;
  if (first_frame_) {
    first_frame_ = false;
    return;
  }
  if (cmd.type == InputType::kQuit) {
    if (game_) game_->Quit();
    return;
  }
  if (cmd.type == InputType::kNone) return;
  if (current_line_index_ + 1 < lines_.size()) {
    ++current_line_index_;
    RenderSystem::SetDialogueText(npc_name_,
                                  {lines_[current_line_index_].text});
  } else {
    finished_ = true;
    RenderSystem::SetDialogueText("Silence", {"..."});
    game_->PopState();
  }
}

void DialogueState::Update(float /*delta*/, DataStore& /*data*/) {}

void DialogueState::Draw(const DataStore& data) {
  int player_idx = data.GetPlayer().entity_index;
  RenderSystem::DrawExploration(data, player_idx);
}

}  // namespace kernel
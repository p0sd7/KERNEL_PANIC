// dialogue_state.cpp
#include "dialogue_state.h"

#include "../DataStore/data_store.h"
#include "../Logging/logger.h"
#include "../RenderSystem/console_renderer.h"
#include "../game_constants.h"
#include "game.h"

namespace kernel {

DialogueState::DialogueState(DataStore& data, int npc_id) {
  npc_name_ = data.getNpcName(npc_id);
  lines_ = data.getDialoguesForNpc(npc_id, data.getMemoryPercent(),
                                   data.getFragments());
}

void DialogueState::handleInput(const InputCommand& cmd, DataStore& data,
                                ConsoleRenderer& renderer) {
  if (finished_) return;
  if (first_frame_) {
    first_frame_ = false;
    if (!lines_.empty() && npc_name_ == "healer.dll") {
      int new_mem =
          data.getPlayer().memory_percent + game_constants::kHealerMemoryBonus;
      if (!data.hasScriptInInventory(game_constants::kHealerScriptId)) {
        data.addScriptToInventory(game_constants::kHealerScriptId);
        data.setMemoryPercent(new_mem);
      }
    }
    if (!lines_.empty()) {
      renderer.setDialogueText(npc_name_, {lines_[0].text});
    } else {
      renderer.setDialogueText(npc_name_, {"..."});
    }
    return;
  }
  if (cmd.type == InputType::kQuit) {
    if (game_) game_->quit();
    return;
  }
  if (cmd.type == InputType::kNone) return;
  if (current_line_index_ + 1 < lines_.size()) {
    ++current_line_index_;
    renderer.setDialogueText(npc_name_, {lines_[current_line_index_].text});
  } else {
    finished_ = true;
    renderer.setDialogueText("Silence", {"..."});
    game_->popState();
  }
}

void DialogueState::update(float /*delta*/, DataStore& /*data*/) {}

void DialogueState::draw(const DataStore& data, ConsoleRenderer& renderer) {
  renderer.drawExploration(data, data.getPlayer().entity_index);
}

}  // namespace kernel
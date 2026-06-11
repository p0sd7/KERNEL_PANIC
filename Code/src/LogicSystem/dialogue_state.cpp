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
  // Игнорируем команды движения и текстового ввода – только подтверждение
  // (любая клавиша, кроме None)
  if (cmd.type == InputType::kNone) return;
  // Теперь любое другое нажатие (kConfirm, kMove, kTextInput) пролистывает
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
  int player_idx = data.GetPlayerIndex();
  RenderSystem::DrawExploration(data, player_idx);
}

}  // namespace kernel
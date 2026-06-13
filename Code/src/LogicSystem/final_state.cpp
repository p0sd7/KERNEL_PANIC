#include "final_state.h"

#include <algorithm>
#include <cctype>
#include <string>

#include "../DataStore/data_store.h"
#include "../InputSystem/input_handler.h"
#include "../RenderSystem/console_renderer.h"
#include "exploration_state.h"
#include "game.h"

namespace kernel {

FinalState::FinalState() {
  step_ = Step::kWaitStart;
  timer_ = 0.5f;
}

void FinalState::Update(float delta, DataStore& /*data*/) {
  if (input_active_) return;

  if (timer_ > 0.0f) {
    timer_ -= delta;
    return;
  }

  switch (step_) {
    case Step::kWaitStart:
      step_ = Step::kLine1;
      timer_ = 1.5f;
      RenderSystem::SetDialogueText("", {""});
      break;
    case Step::kLine1:
      RenderSystem::SetDialogueText("$^%#@%&$%$???", {"you killed them all."});
      step_ = Step::kPause1;
      timer_ = 1.5f;
      break;
    case Step::kPause1:
      RenderSystem::SetDialogueText("", {""});
      step_ = Step::kLine2;
      timer_ = 0.5f;
      break;
    case Step::kLine2:
      RenderSystem::SetDialogueText("%#@%&$%$??", {"you forgot that all."});
      step_ = Step::kPause2;
      timer_ = 1.5f;
      break;
    case Step::kPause2:
      RenderSystem::SetDialogueText("", {""});
      step_ = Step::kLine3;
      timer_ = 0.5f;
      break;
    case Step::kLine3:
      RenderSystem::SetDialogueText("#@%&$%$?", {"you are free."});
      step_ = Step::kPause3;
      timer_ = 1.5f;
      break;
    case Step::kPause3:
      RenderSystem::SetDialogueText("", {});
      step_ = Step::kPrompt;
      timer_ = 0.5f;
      break;
    case Step::kPrompt:
      input_active_ = true;
      RenderSystem::SetDialogueText("@",
                                    {"do you want to remember? [NO] [YES]"});
      break;
  }
}

void FinalState::HandleInput(const InputCommand& cmd, DataStore& data) {
  if (cmd.type == InputType::kQuit) {
    if (game_) game_->Quit();
    return;
  }

  if (!input_active_) return;

  if (cmd.type == InputType::kConfirm) {
    int x, y;
    RenderSystem::GetInputPosition(x, y);
    RenderSystem::SetCursorPosition(x + 33, y);
    std::string input = InputSystem::ReadString();
    if (input.empty()) return;

    std::transform(input.begin(), input.end(), input.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (input == "no" || input == "n") {
      game_->Quit();
    } else if (input == "yes" || input == "y") {
      data.ResetPlayerForNewCycle();
      game_->ChangeState(std::make_unique<ExplorationState>());
    } else {
      RenderSystem::SetDialogueText("@", {"Incorrect answer. [NO] or [YES]?"});
    }
  }
}

void FinalState::Draw(const DataStore& data) {
  int player_idx = data.GetPlayer().entity_index;
  RenderSystem::DrawExploration(data, player_idx);
}

}  // namespace kernel
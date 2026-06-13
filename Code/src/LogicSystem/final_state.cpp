#include "final_state.h"

#include <algorithm>
#include <cctype>

#include "../DataStore/data_store.h"
#include "../InputSystem/input_handler.h"
#include "../RenderSystem/console_renderer.h"
#include "exploration_state.h"
#include "game.h"

namespace kernel {

FinalState::FinalState() : timer_(0.5f) {}

void FinalState::update(float delta, DataStore& /*data*/) {
  if (input_active_) return;

  timer_ -= delta;
  if (timer_ > 0.0f) return;

  switch (step_) {
    case Step::kWaitStart:
      step_ = Step::kLine1;
      timer_ = 1.5f;
      break;
    case Step::kLine1:
      step_ = Step::kPause1;
      timer_ = 1.5f;
      break;
    case Step::kPause1:
      step_ = Step::kLine2;
      timer_ = 0.5f;
      break;
    case Step::kLine2:
      step_ = Step::kPause2;
      timer_ = 1.5f;
      break;
    case Step::kPause2:
      step_ = Step::kLine3;
      timer_ = 0.5f;
      break;
    case Step::kLine3:
      step_ = Step::kPause3;
      timer_ = 1.5f;
      break;
    case Step::kPause3:
      step_ = Step::kPrompt;
      input_active_ = true;
      break;
    case Step::kPrompt:
      break;
  }
}

void FinalState::handleInput(const InputCommand& cmd, DataStore& data,
                             ConsoleRenderer& renderer) {
  if (cmd.type == InputType::kQuit) {
    if (game_) game_->quit();
    return;
  }

  if (input_active_ && cmd.type == InputType::kConfirm) {
    renderer.flushInput();
    int row, col;
    renderer.getInputPosition(row, col);
    renderer.setCursorPosition(row + 21, col + 40);
    std::string input = InputSystem::readString();
    if (input.empty()) return;

    std::transform(input.begin(), input.end(), input.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (input == "no" || input == "n") {
      game_->quit();
    } else if (input == "yes" || input == "y") {
      data.reloadAssets();
      data.resetPlayerForNewCycle();
      renderer.setDialogueText("Silence", {"..."});
      game_->changeState(std::make_unique<ExplorationState>());
    }
  }
}

void FinalState::draw(const DataStore& data, ConsoleRenderer& renderer) {
  switch (step_) {
    case Step::kWaitStart:
      renderer.setDialogueText("", {""});
      break;
    case Step::kLine1:
      renderer.setDialogueText("$^%#@%&$%$???", {"you killed them all."});
      break;
    case Step::kPause1:
      renderer.setDialogueText("", {""});
      break;
    case Step::kLine2:
      renderer.setDialogueText("%#@%&$%$??", {"you forgot that all."});
      break;
    case Step::kPause2:
      renderer.setDialogueText("", {""});
      break;
    case Step::kLine3:
      renderer.setDialogueText("#@%&$%$?", {"you are free."});
      break;
    case Step::kPause3:
      renderer.setDialogueText("", {""});
      break;
    case Step::kPrompt:
      renderer.setDialogueText("@", {"do you want to remember? [NO] [YES]"});
      break;
  }

  renderer.drawExploration(data, data.getPlayer().entity_index);
}

}  // namespace kernel
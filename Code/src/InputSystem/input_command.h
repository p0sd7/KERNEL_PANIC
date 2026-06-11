#pragma once

#include <string>

namespace kernel {

enum class InputType {
  kNone,
  kMove,
  kTextInput,
  kConfirm,
  kQuit,
};

struct InputCommand {
  InputType type = InputType::kQuit;
  int dx = 0;
  int dy = 0;
  std::string text;
};

}  // namespace kernel
#include "input_handler.h"

#include <ncurses.h>

#include <string>

namespace kernel {
namespace InputSystem {

InputCommand PollEvents() {
  int ch = getch();
  if (ch == ERR) {
    return InputCommand{InputType::kNone, 0, 0, ""};
  }
  InputCommand cmd;
  switch (ch) {
    case KEY_UP:
    case 'w':
      cmd = {InputType::kMove, 0, -1};
      break;
    case KEY_DOWN:
    case 's':
      cmd = {InputType::kMove, 0, 1};
      break;
    case KEY_LEFT:
    case 'a':
      cmd = {InputType::kMove, -1, 0};
      break;
    case KEY_RIGHT:
    case 'd':
      cmd = {InputType::kMove, 1, 0};
      break;
    case '\n':
      cmd = {InputType::kTextInput, 0, 0, ""};  // потом отдельно обрабатываем
      break;
    case 'q':
    case 'Q':
      cmd = {InputType::kQuit};
      break;
    default:
      cmd = {InputType::kConfirm};
      break;
  }
  return cmd;
}

}  // namespace InputSystem
}  // namespace kernel
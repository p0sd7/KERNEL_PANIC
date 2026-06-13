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
      cmd = {InputType::kConfirm, 0, 0, ""};
      break;
    case 'q':
    case 'Q':
      cmd = {InputType::kQuit};
      break;
    case 'h':
    case 'H':
      cmd = {InputType::kHelp};
      break;
    default:
      cmd = {InputType::kNone};
      break;
  }
  return cmd;
}

std::string ReadString() {
  nodelay(stdscr, FALSE);
  echo();
  curs_set(1);
  char buf[256];
  getstr(buf);
  noecho();
  curs_set(0);
  nodelay(stdscr, TRUE);
  return std::string(buf);
}

}  // namespace InputSystem
}  // namespace kernel
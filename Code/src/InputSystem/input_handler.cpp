#include "input_handler.h"

#include <ncurses.h>

#include <string>

namespace kernel {
namespace InputSystem {

InputCommand pollEvents() {
  int ch = getch();
  if (ch == ERR) {
    return InputCommand{InputType::kNone, 0, 0, ""};
  }
  switch (ch) {
    case KEY_UP:
    case 'w':
      return {InputType::kMove, 0, -1};
    case KEY_DOWN:
    case 's':
      return {InputType::kMove, 0, 1};
    case KEY_LEFT:
    case 'a':
      return {InputType::kMove, -1, 0};
    case KEY_RIGHT:
    case 'd':
      return {InputType::kMove, 1, 0};
    case '\n':
      return {InputType::kConfirm, 0, 0, ""};
    case 'q':
    case 'Q':
      return {InputType::kQuit};
    case 'h':
    case 'H':
      return {InputType::kHelp};
    default:
      return {InputType::kNone};
  }
}

std::string readString() {
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
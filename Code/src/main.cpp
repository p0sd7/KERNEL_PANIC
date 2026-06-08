#include <ncurses.h>

int main() {
  initscr();  // Инициализация
  printw("Hello World!");
  refresh();  // БЕЗ ЭТОГО НИЧЕГО НЕ УВИДИШЬ!
  getch();    // Ждём нажатие
  endwin();   // Завершение
  return 0;
}
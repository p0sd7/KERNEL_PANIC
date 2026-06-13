#include "console_renderer.h"

#include <ncurses.h>

#include <chrono>
#include <clocale>
#include <fstream>

namespace kernel {

void ConsoleRenderer::init(const InterfaceConfig& config) {
  setlocale(LC_ALL, "");
  screen_width_ = config.screen_width;
  screen_height_ = config.screen_height;
  map_width_ = config.map_width;
  map_height_ = config.map_height;
  map_offset_x_ = config.map_offset_x;
  map_offset_y_ = config.map_offset_y;
  inventory_x_ = config.inventory_x;
  inventory_y_ = config.inventory_y;
  inventory_width_ = config.inventory_width;
  dialog_y_ = config.dialog_y;
  dialog_height_ = config.dialog_height;
  bar_width_ = config.bar_width;

  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  nodelay(stdscr, TRUE);
}

void ConsoleRenderer::shutdown() { endwin(); }

void ConsoleRenderer::flushInput() { flushinp(); }

void ConsoleRenderer::clear() { ::clear(); }

void ConsoleRenderer::present() { refresh(); }

void ConsoleRenderer::drawHelpScreen() {
  ::clear();
  int y = 2;
  for (const auto& line : help_lines_) {
    if (y >= screen_height_ - 2) break;
    mvprintw(y++, 2, "%s", line.c_str());
  }
  mvprintw(screen_height_ - 2, 2, "Press H again to close help.");
}

void ConsoleRenderer::loadHelpText(const std::string& path) {
  std::ifstream file(path);
  help_lines_.clear();
  if (!file.is_open()) {
    help_lines_.push_back("Help file not found.");
    return;
  }
  std::string line;
  while (std::getline(file, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    help_lines_.push_back(line);
  }
}

void ConsoleRenderer::toggleHelp() { help_visible_ = !help_visible_; }

bool ConsoleRenderer::isHelpVisible() const { return help_visible_; }

void ConsoleRenderer::drawLocationName(const std::string& name) {
  int x = map_offset_x_ + map_width_ + 2;
  int y = map_offset_y_ - 3;
  mvprintw(y, x, "%s", name.c_str());
}

void ConsoleRenderer::drawTopBar(int hp, int max_hp, int memory) {
  mvprintw(0, 0, "HP: %3d%% [", hp, max_hp);
  int hp_filled = (hp * bar_width_) / max_hp;
  for (int i = 0; i < bar_width_; ++i) addch(i < hp_filled ? '#' : '-');
  printw("]");
  mvprintw(1, 0, "ME: %3d%% [", memory);
  int mem_filled = (memory * bar_width_) / 100;
  for (int i = 0; i < bar_width_; ++i) addch(i < mem_filled ? '#' : '-');
  printw("]");
}

void ConsoleRenderer::setDialogueText(const std::string& target,
                                      const std::vector<std::string>& lines) {
  dialog_target_ = target;
  dialog_lines_ = lines;
  dialog_timer_active_ = false;
}

void ConsoleRenderer::setTemporaryDialogue(
    const std::string& target, const std::vector<std::string>& lines,
    int seconds) {
  setDialogueText(target, lines);
  dialog_timer_active_ = true;
  dialog_timer_start_ = std::chrono::steady_clock::now();
  dialog_timer_seconds_ = seconds;
}

void ConsoleRenderer::setCombatDialogue(const std::string& target,
                                        const std::vector<std::string>& lines) {
  combat_dialog_target_ = target;
  combat_dialog_lines_ = lines;
}

void ConsoleRenderer::drawExploration(const DataStore& data, int player_idx) {
  int loc_id = data.getPlayer().location_id;
  const auto* location = data.getLocationById(loc_id);
  std::string loc_name = location ? location->name : "unknown";

  if (help_visible_) {
    drawHelpScreen();
    return;
  }

  drawLocationName(loc_name);
  if (!location) return;

  std::vector<std::string> grid(map_height_, std::string(map_width_, '.'));
  const auto& bg = data.getBackground(loc_id);
  if (!bg.lines.empty()) grid = bg.lines;

  drawInventory(data, player_idx);

  if (dialog_timer_active_) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                       now - dialog_timer_start_)
                       .count();
    if (elapsed >= dialog_timer_seconds_) {
      setDialogueText("Silence", {"..."});
    }
  }
  drawDialogue(dialog_target_, dialog_lines_);

  const auto& objects = data.getMapObjects(loc_id);
  for (const auto& obj : objects) {
    if (obj.type == "player") continue;
    int x = obj.x, y = obj.y;
    if (x >= 0 && x < map_width_ && y >= 0 && y < map_height_)
      grid[y][x] = obj.symbol;
  }

  const Entity* player = data.getEntity(player_idx);
  if (player) {
    int px = player->position().x, py = player->position().y;
    if (px >= 0 && px < map_width_ && py >= 0 && py < map_height_)
      grid[py][px] = '@';
  }

  for (int y = 0; y < map_height_; ++y)
    mvprintw(map_offset_y_ + y, map_offset_x_, "%s", grid[y].c_str());

  int hp = player ? player->getStat(StatType::kHp) : 0;
  int max_hp = player ? player->getStat(StatType::kMaxHp) : 100;
  int memory = data.getMemoryPercent();
  drawTopBar(hp, max_hp, memory);
  input_row_ = 0;
  input_col_ = 0;
}

void ConsoleRenderer::drawInventory(const DataStore& data, int player_idx) {
  const auto& scripts = data.getInventoryScripts();
  mvprintw(inventory_y_, inventory_x_, "Inventory:");
  for (size_t i = 0; i < scripts.size(); ++i) {
    const auto* scr = data.getScriptById(scripts[i]);
    std::string name = scr ? scr->name_for_input : "unknown";
    mvprintw(inventory_y_ + 1 + i, inventory_x_, "- %s", name.c_str());
  }
}

void ConsoleRenderer::drawCombat(const DataStore& data, int player_idx,
                                 const std::vector<int>& enemy_indices,
                                 const std::string& /*log*/,
                                 int highlight_enemy, bool is_boss_fight,
                                 int boss_id) {
  ::clear();
  const Entity* player = data.getEntity(player_idx);
  int hp = player ? player->getStat(StatType::kHp) : 0;
  int max_hp = player ? player->getStat(StatType::kMaxHp) : 100;
  int memory = data.getMemoryPercent();
  drawTopBar(hp, max_hp, memory);

  int start_y = 3;
  int start_x = 2;

  if (is_boss_fight && boss_id != -1) {
    const auto& art = data.getBossArt(boss_id);
    for (size_t i = 0; i < art.size() && start_y + i < screen_height_ - 6;
         ++i) {
      mvprintw(start_y + i, start_x, "%s", art[i].c_str());
    }
    if (!enemy_indices.empty()) {
      const Entity* boss = data.getEntity(enemy_indices[0]);
      if (boss) {
        int hp_curr = boss->getStat(StatType::kHp);
        int hp_max = boss->getStat(StatType::kMaxHp);
        int percent = (hp_max > 0) ? (hp_curr * 100) / hp_max : 0;
        mvprintw(start_y + art.size() + 1, start_x, "BOSS HP: %d%%", percent);
      }
    }
  } else {
    mvprintw(start_y, start_x, "Enemies:");
    for (size_t i = 0; i < enemy_indices.size(); ++i) {
      int idx = enemy_indices[i];
      const Entity* enemy = data.getEntity(idx);
      if (!enemy) continue;
      int enemy_id = enemy->getStat(StatType::kEnemyId);
      const auto* templ = data.getEnemyTemplate(enemy_id);
      std::string name = templ ? templ->name : "Unknown";
      int hp_curr = enemy->getStat(StatType::kHp);
      int hp_max = enemy->getStat(StatType::kMaxHp);
      int percent = (hp_max > 0) ? (hp_curr * 100) / hp_max : 0;
      if (static_cast<int>(i) == highlight_enemy) attron(A_REVERSE);
      mvprintw(start_y + 1, start_x + i * 20, "[%d] %s %d%%", i + 1,
               name.c_str(), percent);
      if (static_cast<int>(i) == highlight_enemy) attroff(A_REVERSE);
    }
  }

  int combat_dialog_y = screen_height_ - 8;
  if (combat_dialog_y > 0 && !combat_dialog_lines_.empty()) {
    for (int i = 0; i < screen_width_ - 4; ++i)
      mvaddch(combat_dialog_y, i, '=');
    mvprintw(combat_dialog_y + 1, 2, "%s", combat_dialog_target_.c_str());
    for (int i = 0; i < screen_width_ - 4; ++i)
      mvaddch(combat_dialog_y + 2, i, '=');
    int line_y = combat_dialog_y + 3;
    for (const auto& line : combat_dialog_lines_) {
      if (line_y >= screen_height_ - 4) break;
      mvprintw(line_y++, 2, "%s", line.c_str());
    }
  }

  input_row_ = screen_height_ - 3;
  input_col_ = 2;
  mvprintw(input_row_, input_col_, "> ");
  move(input_row_, input_col_ + 2);
}

void ConsoleRenderer::drawDialogue(const std::string& target_name,
                                   const std::vector<std::string>& lines) {
  if (dialog_height_ < 3) return;
  for (int i = 0; i < screen_width_; ++i) mvaddch(dialog_y_, i, '=');
  mvprintw(dialog_y_ + 1, 2, "%s", target_name.c_str());
  for (int i = 0; i < screen_width_; ++i) mvaddch(dialog_y_ + 2, i, '=');
  int line_y = dialog_y_ + 3;
  for (const auto& line : lines) {
    if (line_y >= screen_height_) break;
    mvprintw(line_y++, 2, "%s", line.c_str());
  }
}

void ConsoleRenderer::drawGameOver() {
  ::clear();
  attron(COLOR_PAIR(1) | A_BOLD);
  mvprintw(screen_height_ / 2, screen_width_ / 2 - 20,
           "KERNEL_PANIC: fatal error. Reason: you.");
  attroff(COLOR_PAIR(1) | A_BOLD);
  mvprintw(screen_height_ / 2 + 2, screen_width_ / 2 - 10,
           "Press any key to reboot.");
}

void ConsoleRenderer::drawFinal(const std::string& prompt) {
  ::clear();
  mvprintw(screen_height_ / 2 - 2,
           screen_width_ / 2 - static_cast<int>(prompt.length()) / 2, "%s",
           prompt.c_str());
  mvprintw(screen_height_ / 2, screen_width_ / 2 - 10,
           "Do you want to remember? [NO] [YES]");
}

void ConsoleRenderer::getInputPosition(int& row, int& col) const {
  row = input_row_;
  col = input_col_;
}

void ConsoleRenderer::setCursorPosition(int row, int col) {
  move(row, col);
  refresh();
}

}  // namespace kernel
// Copyright 2026, KERNEL_PANIC. All rights reserved.

#include "console_renderer.h"

#include <ncurses.h>

#include <chrono>
#include <clocale>
#include <string>
#include <vector>

#include "../DataStore/data_store.h"
#include "../Entities/entity.h"

namespace kernel {
namespace RenderSystem {

static int screen_width_;
static int screen_height_;
static int map_width_;
static int map_height_;
static int map_offset_x_;
static int map_offset_y_;
static int inventory_y_;
static int inventory_x_;
static int inventory_width_;
static int dialog_y_;
static int dialog_height_;
static int bar_width_;

static int input_y_ = 0;
static int input_x_ = 0;

static std::chrono::steady_clock::time_point g_dialog_timer_start;
static bool g_dialog_timer_active = false;
static int g_dialog_timer_seconds = 5;

static std::vector<std::string> g_dialog_lines = {"..."};
static std::string g_dialog_target = "Silence";

static std::vector<std::string> g_combat_dialog_lines;
static std::string g_combat_dialog_target;

void Init(const InterfaceConfig& config) {
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

void Shutdown() { endwin(); }

void FlushInput() { flushinp(); }

void Clear() { ::clear(); }

void DrawLocationName(const std::string& name) {
  int x = map_offset_x_ + map_width_ + 2;
  int y = map_offset_y_ - 3;
  mvprintw(y, x, "%s", name.c_str());
}

void DrawTopBar(int hp, int max_hp, int memory) {
  mvprintw(0, 0, "HP: %3d%% [", hp, max_hp);
  int hp_filled = (hp * bar_width_) / max_hp;
  for (int i = 0; i < bar_width_; ++i) addch(i < hp_filled ? '#' : '-');
  printw("]");
  mvprintw(1, 0, "ME: %3d%% [", memory);
  int mem_filled = (memory * bar_width_) / 100;
  for (int i = 0; i < bar_width_; ++i) addch(i < mem_filled ? '#' : '-');
  printw("]");
}

void SetDialogueText(const std::string& target,
                     const std::vector<std::string>& lines) {
  g_dialog_target = target;
  g_dialog_lines = lines;
  g_dialog_timer_active = false;
}

void SetTemporaryDialogue(const std::string& target,
                          const std::vector<std::string>& lines, int seconds) {
  SetDialogueText(target, lines);
  g_dialog_timer_active = true;
  g_dialog_timer_start = std::chrono::steady_clock::now();
  g_dialog_timer_seconds = seconds;
}

void SetCombatDialogue(const std::string& target,
                       const std::vector<std::string>& lines) {
  g_combat_dialog_target = target;
  g_combat_dialog_lines = lines;
}

void DrawExploration(const DataStore& data, int player_idx) {
  int loc_id = data.GetPlayer().location_id;
  const auto* location = data.GetLocationById(loc_id);
  std::string loc_name = location ? location->name : "unknown";
  DrawLocationName(loc_name);
  if (!location) return;

  std::vector<std::string> grid(map_height_, std::string(map_width_, '.'));
  const auto& bg = data.GetBackground(loc_id);
  if (!bg.lines.empty()) grid = bg.lines;

  DrawInventory(data, player_idx);
  if (g_dialog_timer_active) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                       now - g_dialog_timer_start)
                       .count();
    if (elapsed >= g_dialog_timer_seconds) {
      SetDialogueText("Silence", {"..."});
    }
  }
  DrawDialogue(g_dialog_target, g_dialog_lines);

  const auto& objects = data.GetMapObjects(loc_id);
  for (const auto& obj : objects) {
    if (obj.type == "player") continue;
    int x = obj.x, y = obj.y;
    if (x >= 0 && x < map_width_ && y >= 0 && y < map_height_)
      grid[y][x] = obj.symbol;
  }

  const Entity* player = data.GetEntity(player_idx);
  if (player) {
    int px = player->Position().x, py = player->Position().y;
    if (px >= 0 && px < map_width_ && py >= 0 && py < map_height_)
      grid[py][px] = '@';
  }

  for (int y = 0; y < map_height_; ++y)
    mvprintw(map_offset_y_ + y, map_offset_x_, "%s", grid[y].c_str());

  int hp = player ? player->GetStat(StatType::kHp) : 0;
  int max_hp = player ? player->GetStat(StatType::kMaxHp) : 100;
  int memory = data.GetMemoryPercent();
  DrawTopBar(hp, max_hp, memory);
}

void DrawInventory(const DataStore& data, int player_idx) {
  const auto& scripts = data.GetInventoryScripts();
  mvprintw(inventory_y_, inventory_x_, "Inventory:");
  for (size_t i = 0; i < scripts.size(); ++i) {
    const auto* scr = data.GetScriptById(scripts[i]);
    std::string name = scr ? scr->name_for_input : "unknown";
    mvprintw(inventory_y_ + 1 + i, inventory_x_, "- %s", name.c_str());
  }
}

void DrawCombat(const DataStore& data, int player_idx,
                const std::vector<int>& enemy_indices,
                const std::string& /*log*/, int highlight_enemy,
                bool is_boss_fight, int boss_id) {
  clear();

  const Entity* player = data.GetEntity(player_idx);
  int hp = player ? player->GetStat(StatType::kHp) : 0;
  int max_hp = player ? player->GetStat(StatType::kMaxHp) : 100;
  int memory = data.GetMemoryPercent();
  DrawTopBar(hp, max_hp, memory);

  int start_y = 3;
  int start_x = 2;

  if (is_boss_fight && boss_id != -1) {
    const auto& art = data.GetBossArt(boss_id);
    for (size_t i = 0; i < art.size() && start_y + i < screen_height_ - 6;
         ++i) {
      mvprintw(start_y + i, start_x, "%s", art[i].c_str());
    }
    if (!enemy_indices.empty()) {
      const Entity* boss = data.GetEntity(enemy_indices[0]);
      if (boss) {
        int hp_curr = boss->GetStat(StatType::kHp);
        int hp_max = boss->GetStat(StatType::kMaxHp);
        int percent = (hp_max > 0) ? (hp_curr * 100) / hp_max : 0;
        mvprintw(start_y + art.size() + 1, start_x, "BOSS HP: %d%%", percent);
      }
    }
  } else {
    mvprintw(start_y, start_x, "Enemies:");
    for (size_t i = 0; i < enemy_indices.size(); ++i) {
      int idx = enemy_indices[i];
      const Entity* enemy = data.GetEntity(idx);
      if (!enemy) continue;
      int enemy_id = enemy->GetStat(StatType::kEnemyId);
      const auto* templ = data.GetEnemyTemplate(enemy_id);
      std::string name = templ ? templ->name : "Unknown";
      int hp_curr = enemy->GetStat(StatType::kHp);
      int hp_max = enemy->GetStat(StatType::kMaxHp);
      int percent = (hp_max > 0) ? (hp_curr * 100) / hp_max : 0;
      if (static_cast<int>(i) == highlight_enemy) attron(A_REVERSE);
      mvprintw(start_y + 1, start_x + i * 20, "[%d] %s %d%%", i + 1,
               name.c_str(), percent);
      if (static_cast<int>(i) == highlight_enemy) attroff(A_REVERSE);
    }
  }
  int combat_dialog_y = screen_height_ - 8;
  int dialog_width = screen_width_ - 4;
  if (combat_dialog_y > 0 && !g_combat_dialog_lines.empty()) {
    for (int i = 0; i < dialog_width; ++i) mvaddch(combat_dialog_y, i, '=');
    mvprintw(combat_dialog_y + 1, 2, "%s", g_combat_dialog_target.c_str());
    for (int i = 0; i < dialog_width; ++i) mvaddch(combat_dialog_y + 2, i, '=');
    int line_y = combat_dialog_y + 3;
    for (const auto& line : g_combat_dialog_lines) {
      if (line_y >= screen_height_ - 4) break;
      mvprintw(line_y++, 2, "%s", line.c_str());
    }
  }

  input_y_ = screen_height_ - 3;
  input_x_ = 2;
  mvprintw(input_y_, input_x_, "> ");
  move(input_y_, input_x_ + 2);
  refresh();
}

void DrawDialogue(const std::string& target_name,
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

void DrawGameOver() {
  clear();
  attron(COLOR_PAIR(1) | A_BOLD);
  mvprintw(screen_height_ / 2, screen_width_ / 2 - 20,
           "KERNEL_PANIC: fatal error. Reason: you.");
  attroff(COLOR_PAIR(1) | A_BOLD);
  mvprintw(screen_height_ / 2 + 2, screen_width_ / 2 - 10,
           "Press any key to reboot.");
}

void DrawFinal(const std::string& prompt) {
  clear();
  mvprintw(screen_height_ / 2 - 2,
           screen_width_ / 2 - static_cast<int>(prompt.length()) / 2, "%s",
           prompt.c_str());
  mvprintw(screen_height_ / 2, screen_width_ / 2 - 10,
           "Do you want to remember? [NO] [YES]");
}

void Present() { refresh(); }

void GetInputPosition(int& y, int& x) {
  y = screen_height_ - 3;
  x = 4;
}

void SetCursorPosition(int y, int x) {
  move(y, x);
  refresh();
}

}  // namespace RenderSystem
}  // namespace kernel
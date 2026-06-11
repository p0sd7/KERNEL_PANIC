// Copyright 2026, KERNEL_PANIC. All rights reserved.

#include "console_renderer.h"

#include <ncurses.h>

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

static std::vector<std::string> g_dialog_lines = {"..."};
static std::string g_dialog_target = "Silence";

void Init(const InterfaceConfig& config) {
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

void Clear() { ::clear(); }

void DrawLocationName(const std::string& name) {
  int x = map_offset_x_ + map_width_ + 2;
  int y = map_offset_y_ - 3;
  mvprintw(y, x, "%s", name.c_str());
}

void DrawTopBar(int hp, int max_hp, int memory) {
  mvprintw(0, 0, "HP: %3d/%3d [", hp, max_hp);
  int hp_filled = (hp * bar_width_) / max_hp;
  for (int i = 0; i < bar_width_; ++i) addch(i < hp_filled ? '#' : '-');
  printw("]");
  mvprintw(1, 0, "MEM: %3d%% [", memory);
  int mem_filled = (memory * bar_width_) / 100;
  for (int i = 0; i < bar_width_; ++i) addch(i < mem_filled ? '#' : '-');
  printw("]");
}

void SetDialogueText(const std::string& target,
                     const std::vector<std::string>& lines) {
  g_dialog_target = target;
  g_dialog_lines = lines;
}

void DrawExploration(const DataStore& data, int player_idx) {
  int loc_id = data.GetPlayerLocationId();
  const auto* location = data.GetLocationById(loc_id);
  std::string loc_name = location ? location->name : "unknown";
  DrawLocationName(loc_name);
  if (!location) return;

  std::vector<std::string> grid(map_height_, std::string(map_width_, '.'));
  const auto& bg = data.GetBackground(loc_id);
  if (!bg.lines.empty()) grid = bg.lines;

  DrawInventory(data, player_idx);
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
                const std::vector<int>& enemy_indices, const std::string& log) {
  (void)data;
  (void)player_idx;
  (void)enemy_indices;
  (void)log;
  clear();
  mvprintw(0, 0, "COMBAT MODE");
  mvprintw(2, 0, "Log: %s", log.c_str());
  mvprintw(5, 0, "Enter script: ");
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

}  // namespace RenderSystem
}  // namespace kernel
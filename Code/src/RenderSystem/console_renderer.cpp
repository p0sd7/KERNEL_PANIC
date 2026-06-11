// Copyright 2026, KERNEL_PANIC. All rights reserved.

#include "console_renderer.h"

#include <ncurses.h>

#include <string>
#include <vector>

#include "../DataStore/data_store.h"
#include "../Entities/entity.h"

namespace kernel {
namespace RenderSystem {

static int kScreenWidth;
static int kScreenHeight;
static int kMapWidth;
static int kMapHeight;
static int kMapOffsetX;
static int kMapOffsetY;
static int kInventoryY;
static int kInventoryX;
static int kInventoryWidth;
static int kDialogY;
static int kDialogHeight;
static int kBarWidth;

static std::vector<std::string> g_dialog_lines = {"..."};
static std::string g_dialog_target = "Silence";

void Init(const InterfaceConfig& config) {
  kScreenWidth = config.screen_width;
  kScreenHeight = config.screen_height;
  kMapWidth = config.map_width;
  kMapHeight = config.map_height;
  kMapOffsetX = config.map_offset_x;
  kMapOffsetY = config.map_offset_y;
  kInventoryX = config.inventory_x;
  kInventoryY = config.inventory_y;
  kInventoryWidth = config.inventory_width;
  kDialogY = config.dialog_y;
  kDialogHeight = config.dialog_height;
  kBarWidth = config.bar_width;

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
  int x = kMapOffsetX + kMapWidth + 2;
  int y = kMapOffsetY - 3;
  mvprintw(y, x, "%s", name.c_str());
}

void DrawTopBar(int hp, int max_hp, int memory) {
  int hp_percent = (hp * 100) / max_hp;
  int mem_percent = memory;

  mvprintw(0, 0, "HP: %3d%% [", hp, max_hp);
  int hp_filled = (hp * kBarWidth) / max_hp;
  for (int i = 0; i < kBarWidth; ++i) {
    addch(i < hp_filled ? '#' : '-');
  }
  printw("]");

  mvprintw(1, 0, "ME: %3d%% [", mem_percent);
  int mem_filled = (mem_percent * kBarWidth) / 100;
  for (int i = 0; i < kBarWidth; ++i) {
    addch(i < mem_filled ? '#' : '-');
  }
  printw("]");
}

void SetDialogText(const std::string& target,
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
  std::vector<std::string> grid(kMapHeight, std::string(kMapWidth, '.'));
  const auto& bg = data.GetBackground(loc_id);
  if (!bg.lines.empty()) {
    grid = bg.lines;
  }
  DrawInventory(data, player_idx);

  DrawDialog(g_dialog_target, g_dialog_lines);

  const auto& objects = data.GetMapObjects(loc_id);
  for (const auto& obj : objects) {
    if (obj.type == "player") continue;
    int x = obj.x, y = obj.y;
    if (x >= 0 && x < kMapWidth && y >= 0 && y < kMapHeight) {
      if (y < static_cast<int>(grid.size()) &&
          x < static_cast<int>(grid[y].size())) {
        grid[y][x] = obj.symbol;
      }
    }
  }

  const Entity* player = data.GetEntity(player_idx);
  if (player) {
    int px = player->Position().x;
    int py = player->Position().y;
    if (px >= 0 && px < kMapWidth && py >= 0 && py < kMapHeight) {
      if (py < static_cast<int>(grid.size()) &&
          px < static_cast<int>(grid[py].size())) {
        grid[py][px] = '@';
      }
    }
  }

  for (int y = 0; y < kMapHeight; ++y) {
    mvprintw(kMapOffsetY + y, kMapOffsetX, "%s", grid[y].c_str());
  }
  int hp = player ? player->GetStat(StatType::kHp) : 0;
  int max_hp = player ? player->GetStat(StatType::kMaxHp) : 100;
  int memory = data.GetMemoryPercent();
  DrawTopBar(hp, max_hp, memory);
}

void DrawInventory(const DataStore& data, int player_idx) {
  const auto& scripts = data.GetInventoryScripts();
  mvprintw(kInventoryY, kInventoryX, "Inventory:");
  for (size_t i = 0; i < scripts.size(); ++i) {
    const auto* scr = data.GetScriptById(scripts[i]);
    std::string name = scr ? scr->name_for_input : "unknown";
    mvprintw(kInventoryY + 1 + i, kInventoryX, "- %s", name.c_str());
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

void DrawDialog(const std::string& target_name,
                const std::vector<std::string>& lines) {
  if (kDialogHeight < 3) return;

  // Рисуем рамку из '=' (первая строка)
  for (int i = 0; i < kScreenWidth; ++i) {
    mvaddch(kDialogY, i, '=');
  }
  // Вторая строка: имя цели
  mvprintw(kDialogY + 1, 2, "%s", target_name.c_str());
  // Третья строка: снова '='
  for (int i = 0; i < kScreenWidth; ++i) {
    mvaddch(kDialogY + 2, i, '=');
  }
  // Далее текст (с отступом)
  int line_y = kDialogY + 3;
  for (const auto& line : lines) {
    if (line_y >= kScreenHeight) break;
    mvprintw(line_y++, 2, "%s", line.c_str());
  }
}

void DrawGameOver() {
  clear();
  // Синий фон (если поддерживается, иначе просто текст)
  attron(COLOR_PAIR(1) | A_BOLD);
  mvprintw(kScreenHeight / 2, kScreenWidth / 2 - 20,
           "KERNEL_PANIC: fatal error. Reason: you.");
  attroff(COLOR_PAIR(1) | A_BOLD);
  mvprintw(kScreenHeight / 2 + 2, kScreenWidth / 2 - 10,
           "Press any key to reboot.");
}

void DrawFinal(const std::string& prompt) {
  clear();
  mvprintw(kScreenHeight / 2 - 2,
           kScreenWidth / 2 - static_cast<int>(prompt.length()) / 2, "%s",
           prompt.c_str());
  mvprintw(kScreenHeight / 2, kScreenWidth / 2 - 10,
           "Do you want to remember? [NO] [YES]");
}

void Present() { refresh(); }

}  // namespace RenderSystem
}  // namespace kernel
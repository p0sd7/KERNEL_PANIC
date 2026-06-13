#pragma once

#include <string>
#include <vector>

#include "../DataStore/data_store.h"
#include "../Entities/entity.h"

namespace kernel {

class DataStore;

class ConsoleRenderer {
 public:
  void init(const InterfaceConfig& config);
  void shutdown();
  void clear();
  void present();

  void drawExploration(const DataStore& data, int player_idx);
  void drawInventory(const DataStore& data, int player_idx);
  void drawCombat(const DataStore& data, int player_idx,
                  const std::vector<int>& enemy_indices, const std::string& log,
                  int highlight_enemy = -1, bool is_boss_fight = false,
                  int boss_id = -1);
  void drawDialogue(const std::string& target_name,
                    const std::vector<std::string>& lines);
  void drawTopBar(int hp, int max_hp, int memory);
  void drawLocationName(const std::string& name);
  void drawGameOver();
  void drawFinal(const std::string& prompt);

  void setDialogueText(const std::string& target,
                       const std::vector<std::string>& lines);
  void setCombatDialogue(const std::string& target,
                         const std::vector<std::string>& lines);
  void setTemporaryDialogue(const std::string& target,
                            const std::vector<std::string>& lines,
                            int seconds = 3);

  void flushInput();
  void getInputPosition(int& row, int& col) const;  // row, col
  void setCursorPosition(int row, int col);

  void loadHelpText(const std::string& path);
  void toggleHelp();
  bool isHelpVisible() const;

 private:
  void drawHelpScreen();

  int screen_width_ = 80;
  int screen_height_ = 24;
  int map_width_ = 40;
  int map_height_ = 15;
  int map_offset_x_ = 2;
  int map_offset_y_ = 3;
  int inventory_y_ = 2;
  int inventory_x_ = 44;
  int inventory_width_ = 20;
  int dialog_y_ = 18;
  int dialog_height_ = 5;
  int bar_width_ = 30;

  int input_row_ = 0;
  int input_col_ = 0;

  // dialogue state
  std::chrono::steady_clock::time_point dialog_timer_start_;
  bool dialog_timer_active_ = false;
  int dialog_timer_seconds_ = 5;

  std::vector<std::string> dialog_lines_ = {"..."};
  std::string dialog_target_ = "Silence";

  std::vector<std::string> combat_dialog_lines_;
  std::string combat_dialog_target_;

  // help
  std::vector<std::string> help_lines_;
  bool help_visible_ = false;
};

}  // namespace kernel
#pragma once

#include <string>
#include <vector>

#include "../DataStore/data_store.h"
#include "../Entities/entity.h"

namespace kernel {

class DataStore;

namespace RenderSystem {

void Init(const InterfaceConfig& config);
void Shutdown();
void Clear();
void DrawExploration(const DataStore& data, int player_idx);
void DrawInventory(const DataStore& data, int player_idx);
void DrawCombat(const DataStore& data, int player_idx,
                const std::vector<int>& enemy_indices, const std::string& log,
                int highlight_enemy = -1, bool is_boss_fight = false,
                int boss_id = -1);
void DrawDialogue(const std::string& target_name,
                  const std::vector<std::string>& lines);
void DrawTopBar(int hp, int max_hp, int memory);
void DrawLocationName(const std::string& name);
void DrawGameOver();
void DrawFinal(const std::string& prompt);
void Present();
void SetDialogueText(const std::string& target,
                     const std::vector<std::string>& lines);
void SetCombatDialogue(const std::string& target,
                       const std::vector<std::string>& lines);
void SetTemporaryDialogue(const std::string& target,
                          const std::vector<std::string>& lines,
                          int seconds = 3);
void GetInputPosition(int& y, int& x);
void SetCursorPosition(int y, int x);
void FlushInput();

}  // namespace RenderSystem
}  // namespace kernel
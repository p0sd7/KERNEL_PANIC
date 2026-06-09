#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../Entities/entity.h"

namespace kernel {

struct LocationData {
  int id = -1;
  std::string name;
  std::string ascii_background;
  char exit_symbol = 0;
  bool forced_combat_on_enter = false;
  int next_location_id = -1;
};

struct MapObjectData {
  int id = -1;
  int location_id = -1;
  std::string type;  // "player", "npc", "item", "boss", "trap", "exit"
  char symbol = '?';
  int x = 0, y = 0;
  std::string ref_id;
  std::string special_condition;
};

enum class EnemyType {
  kBoss,
  kRandom,
};

struct EnemyTemplate {
  int id = -1;
  std::string name;
  std::string ascii_display;
  int hp = 0;
  int damage = 0;
  EnemyType enemy_type = EnemyType::kRandom;
  std::string special_ai;
  std::string dialogue_on_spawn;
};

struct DialogueLine {
  int id = -1;
  std::string npc_id;
  int condition_memory_min = 0;
  int condition_memory_max = 100;
  int condition_fragments = -1;
  std::string text;
};

struct ScriptData {
  int id = -1;
  std::string name_for_input;
  int damage = 0;
  int self_damage = 0;
  int defense_percent = 0;
  bool stun_target = false;
  std::string available_after_boss;
};

struct PuzzleData {
  int id = -1;
  int location_id = -1;
  std::string type;
  std::string solution_data;
  std::string reward_item_id;
  std::string wrong_penalty;
};

class DataStore {
 public:
  void LoadAll(const std::string& assets_path);

  int AddEntity(std::unique_ptr<Entity> entity);
  void RemoveEntity(int index);
  Entity* GetEntity(int index);
  const std::vector<std::unique_ptr<Entity>>& GetEntities() const {
    return entities_;
  }

  const LocationData* GetLocationById(int id) const;
  int GetLocationIdByName(const std::string& name) const;
  const std::vector<MapObjectData>& GetMapObjects(int location_id) const;
  const EnemyTemplate* GetEnemyTemplate(int id) const;
  int GetEnemyIdByName(const std::string& name) const;
  const ScriptData* GetScriptById(int id) const;
  int GetScriptIdByName(const std::string& name) const;
  std::vector<DialogueLine> GetDialoguesForNpc(const std::string& npc_id,
                                               int memory, int fragments) const;
  int GetNpcDefaultDialogue(const std::string& npc_id) const;
  int GetScriptIdByItemId(const std::string& item_id) const;
  const std::vector<int>& GetEnemyGroup(int location_id) const;
  const std::vector<std::string>& GetMemoryFragments() const;
  const PuzzleData* GetPuzzleByLocation(int location_id) const;

  void SetPlayerIndex(int idx) { player_index_ = idx; }
  int GetPlayerIndex() const { return player_index_; }

  void AddScriptToInventory(int script_id);
  bool HasScriptInInventory(int script_id) const;
  const std::vector<int>& GetInventoryScripts() const {
    return inventory_script_ids_;
  }
  void SetMemoryPercent(int percent);
  int GetMemoryPercent() const { return memory_percent_; }
  void ResetPlayerForNewCycle();

 private:
  std::vector<std::unique_ptr<Entity>> entities_;
  int player_index_ = -1;

  std::map<int, LocationData> locations_;
  std::map<std::string, int> location_name_to_id_;
  std::map<int, std::vector<MapObjectData>> map_objects_;
  std::map<int, EnemyTemplate> enemy_templates_;
  std::map<std::string, int> enemy_name_to_id_;
  std::map<int, ScriptData> scripts_;
  std::map<std::string, int> script_name_to_id_;
  std::map<int, DialogueLine> dialogues_;
  std::map<std::string, int> npc_default_dialogue_;
  std::map<std::string, int> item_to_script_id_;
  std::map<int, std::vector<int>> enemy_groups_;
  std::vector<std::string> memory_fragments_;
  std::map<int, PuzzleData> puzzles_;

  // потом вынести в отдельную структуру
  std::vector<int> inventory_script_ids_;
  int memory_percent_ = 0;
};

}  // namespace kernel
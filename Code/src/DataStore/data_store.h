#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../Entities/entity.h"

namespace kernel {

struct InterfaceConfig {
  int screen_width = 80;
  int screen_height = 24;
  int map_width = 40;
  int map_height = 15;
  int map_offset_x = 2;
  int map_offset_y = 3;
  int inventory_y = 2;
  int inventory_x = 44;
  int inventory_width = 20;
  int dialog_y = 18;
  int dialog_height = 5;
  int bar_width = 30;
};

struct LocationData {
  int id = -1;
  std::string name;
  std::string ascii_background;
  bool forced_combat_on_enter = false;
  int next_location_id = -1;
};

struct MapObjectData {
  int id = -1;
  int location_id = -1;
  std::string type;  // "player", "npc", "item", "boss", "trap", "exit"
  char symbol = '?';
  int x = 0, y = 0;
  int ref_id = -1;
  std::string special_condition;
};

enum class EnemyType { kBoss, kRandom };

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

struct EnemyGroup {
  int enemy_id = -1;
  int min_count = 1;
  int max_count = 1;
  int spawn_chance = 100;
};

struct DialogueLine {
  int id = -1;
  int npc_id;
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
  int reward_item_id = -1;
  std::string wrong_penalty;
};

enum class ItemType {
  kHeal,
  kScript,
  kTrap,
  kPuzzleItem,
  kMemoryFrag,
  kUnknown
};

struct NpcData {
  int id = -1;
  std::string name;
  int default_dialog_id = -1;
};

struct ItemData {
  int id = -1;
  std::string name;
  ItemType type = ItemType::kUnknown;
  int effect_value = 0;
  int script_id = -1;
};

struct MemoryFragmentData {
  int id = -1;
  std::string text;
};

struct BackgroundData {
  std::vector<std::string> lines;
};

struct PlayerState {
  int entity_index = -1;
  int memory_percent = 0;
  std::vector<int> inventory = {1, 2, 3, 4, 5, 6, 7};
  int fragments_collected = 0;
  int location_id = -1;
};

class DataStore {
 public:
  void LoadAll(const std::string& assets_path);

  int AddEntity(std::unique_ptr<Entity> entity);
  void RemoveEntity(int index);
  Entity* GetEntity(int index) const;
  const std::vector<std::unique_ptr<Entity>>& GetEntities() const {
    return entities_;
  }

  const LocationData* GetLocationById(int id) const;
  int GetLocationIdByName(const std::string& loc_name) const;
  const std::vector<MapObjectData>& GetMapObjects(int location_id) const;
  const EnemyTemplate* GetEnemyTemplate(int id) const;
  const ScriptData* GetScriptById(int id) const;
  int GetScriptIdByName(const std::string& name) const;
  std::vector<DialogueLine> GetDialoguesForNpc(int npc_id, int memory,
                                               int fragments) const;
  const ItemData* GetItemById(int id) const;
  const std::vector<EnemyGroup>& GetEnemyGroup(int location_id) const;
  const std::map<int, MemoryFragmentData>& GetMemoryFragments() const;
  const PuzzleData* GetPuzzleByLocation(int location_id) const;
  const BackgroundData& GetBackground(int location_id) const;

  void RemoveMapObject(int location_id, int object_id);

  PlayerState& GetPlayer() { return player_; }
  const PlayerState& GetPlayer() const { return player_; }

  void AddScriptToInventory(int script_id);
  bool HasScriptInInventory(int script_id) const;
  const std::vector<int>& GetInventoryScripts() const {
    return player_.inventory;
  }
  void SetMemoryPercent(int percent);
  int GetMemoryPercent() const { return player_.memory_percent; }
  void IncrementFragments();
  int GetFragments() const { return player_.fragments_collected; }
  void ResetPlayerForNewCycle();

  const std::string& GetNpcName(int npc_id) const;
  std::pair<int, int> GetSpawnPoint(int loc_id) const;
  const InterfaceConfig& GetInterfaceConfig() const {
    return interface_config_;
  }

  const std::vector<std::string>& GetBossArt(int enemy_id) const;
  const std::map<int, ScriptData>& GetScripts() const { return scripts_; }

 private:
  std::vector<std::unique_ptr<Entity>> entities_;
  PlayerState player_;

  std::map<int, LocationData> locations_;
  std::map<int, std::vector<MapObjectData>> map_objects_;
  std::map<int, BackgroundData> backgrounds_;
  std::map<int, EnemyTemplate> enemy_templates_;
  std::map<int, ScriptData> scripts_;
  std::map<int, DialogueLine> dialogues_;
  std::map<int, NpcData> npc_base_;
  std::map<int, ItemData> items_;
  std::map<int, MemoryFragmentData> memory_fragments_;
  std::map<int, PuzzleData> puzzles_;
  std::map<int, std::vector<EnemyGroup>> enemy_groups_;
  std::map<int, std::vector<std::string>> boss_arts_;

  InterfaceConfig interface_config_;
};

}  // namespace kernel
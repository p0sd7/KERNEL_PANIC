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
  int npc_id = -1;
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
  std::vector<int> inventory;
  int fragments_collected = 0;
  int location_id = -1;
};

class DataStore {
 public:
  void loadAll(const std::string& assets_path);
  void reloadAssets();
  int addEntity(std::unique_ptr<Entity> entity);
  void removeEntity(int index);
  void removeEntities(const std::vector<int>& indices);
  Entity* getEntity(int index) const;
  const std::vector<std::unique_ptr<Entity>>& getEntities() const {
    return entities_;
  }

  const LocationData* getLocationById(int id) const;
  int getLocationIdByName(const std::string& name) const;
  const std::vector<MapObjectData>& getMapObjects(int location_id) const;
  const EnemyTemplate* getEnemyTemplate(int id) const;
  const ScriptData* getScriptById(int id) const;
  int getScriptIdByName(const std::string& name) const;
  std::vector<DialogueLine> getDialoguesForNpc(int npc_id, int memory,
                                               int fragments) const;
  const ItemData* getItemById(int id) const;
  const std::vector<EnemyGroup>& getEnemyGroup(int location_id) const;
  const std::map<int, MemoryFragmentData>& getMemoryFragments() const;
  const PuzzleData* getPuzzleByLocation(int location_id) const;
  const BackgroundData& getBackground(int location_id) const;

  void removeMapObject(int location_id, int object_id);

  PlayerState& getPlayer() { return player_; }
  const PlayerState& getPlayer() const { return player_; }

  void addScriptToInventory(int script_id);
  bool hasScriptInInventory(int script_id) const;
  const std::vector<int>& getInventoryScripts() const;
  void setMemoryPercent(int percent);
  int getMemoryPercent() const;
  void incrementFragments();
  int getFragments() const;
  void resetPlayerForNewCycle();

  const std::string& getNpcName(int npc_id) const;
  std::pair<int, int> getSpawnPoint(int loc_id) const;
  const InterfaceConfig& getInterfaceConfig() const;

  const std::vector<std::string>& getBossArt(int enemy_id) const;
  const std::map<int, ScriptData>& getScripts() const { return scripts_; }

 private:
  std::vector<std::unique_ptr<Entity>> entities_;
  PlayerState player_;
  std::string assets_path_;

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
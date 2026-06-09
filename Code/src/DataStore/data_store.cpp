#include "data_store.h"

#include "../Logging/logger.h"
#include "csv_loader.h"

namespace kernel {

void DataStore::LoadAll(const std::string& assets_path) {
  using logging::LogError;
  using logging::LogInfo;

  std::string loc_path = assets_path + "/locations.csv";
  if (!csv_loader::LoadLocations(loc_path, locations_, location_name_to_id_)) {
    LogError("Failed to load " + loc_path);
  } else {
    LogInfo("Loaded " + loc_path);
  }

  std::string map_path = assets_path + "/map_objects.csv";
  if (!csv_loader::LoadMapObjects(map_path, map_objects_)) {
    LogError("Failed to load " + map_path);
  } else {
    LogInfo("Loaded " + map_path);
  }

  std::string npcs_path = assets_path + "/npcs.csv";
  if (!csv_loader::LoadNpcs(npcs_path, npc_default_dialogue_)) {
    LogError("Failed to load " + npcs_path);
  } else {
    LogInfo("Loaded " + npcs_path);
  }

  std::string dialogues_path = assets_path + "/dialogues.csv";
  if (!csv_loader::LoadDialogues(dialogues_path, dialogues_)) {
    LogError("Failed to load " + dialogues_path);
  } else {
    LogInfo("Loaded " + dialogues_path);
  }

  std::string items_path = assets_path + "/items.csv";
  if (!csv_loader::LoadItems(items_path, item_to_script_id_)) {
    LogError("Failed to load " + items_path);
  } else {
    LogInfo("Loaded " + items_path);
  }

  std::string scripts_path = assets_path + "/scripts.csv";
  if (!csv_loader::LoadScripts(scripts_path, scripts_, script_name_to_id_)) {
    LogError("Failed to load " + scripts_path);
  } else {
    LogInfo("Loaded " + scripts_path);
  }

  std::string enemies_path = assets_path + "/enemies.csv";
  if (!csv_loader::LoadEnemies(enemies_path, enemy_templates_,
                               enemy_name_to_id_)) {
    LogError("Failed to load " + enemies_path);
  } else {
    LogInfo("Loaded " + enemies_path);
  }

  std::string groups_path = assets_path + "/enemy_groups.csv";
  if (!csv_loader::LoadEnemyGroups(groups_path, enemy_groups_)) {
    LogError("Failed to load " + groups_path);
  } else {
    LogInfo("Loaded " + groups_path);
  }

  std::string memory_path = assets_path + "/memory_fragments.csv";
  if (!csv_loader::LoadMemoryFragments(memory_path, memory_fragments_)) {
    LogError("Failed to load " + memory_path);
  } else {
    LogInfo("Loaded " + memory_path);
  }

  std::string puzzles_path = assets_path + "/puzzles.csv";
  if (!csv_loader::LoadPuzzles(puzzles_path, puzzles_)) {
    LogError("Failed to load " + puzzles_path);
  } else {
    LogInfo("Loaded " + puzzles_path);
  }
}

const LocationData* DataStore::GetLocationById(int id) const {
  auto it = locations_.find(id);
  return (it != locations_.end()) ? &it->second : nullptr;
}

int DataStore::GetLocationIdByName(const std::string& name) const {
  auto it = location_name_to_id_.find(name);
  return (it != location_name_to_id_.end()) ? it->second : -1;
}

const std::vector<MapObjectData>& DataStore::GetMapObjects(
    int location_id) const {
  static const std::vector<MapObjectData> empty;
  auto it = map_objects_.find(location_id);
  return (it != map_objects_.end()) ? it->second : empty;
}

int DataStore::AddEntity(std::unique_ptr<Entity> entity) {
  entities_.push_back(std::move(entity));
  return static_cast<int>(entities_.size()) - 1;
}

void DataStore::RemoveEntity(int index) {
  if (index >= 0 && static_cast<size_t>(index) < entities_.size()) {
    entities_.erase(entities_.begin() + index);
  }
}

Entity* DataStore::GetEntity(int index) {
  if (index >= 0 && static_cast<size_t>(index) < entities_.size()) {
    return entities_[index].get();
  }
  return nullptr;
}

void DataStore::AddScriptToInventory(int script_id) {
  inventory_script_ids_.push_back(script_id);
}

bool DataStore::HasScriptInInventory(int script_id) const {
  for (int id : inventory_script_ids_) {
    if (id == script_id) return true;
  }
  return false;
}

void DataStore::SetMemoryPercent(int percent) { memory_percent_ = percent; }

void DataStore::ResetPlayerForNewCycle() {
  inventory_script_ids_.clear();
  memory_percent_ = 0;
  Entity* player = GetEntity(player_index_);
  if (player) {
    player->SetStat(StatType::kHp, player->GetStat(StatType::kMaxHp));
  }
}

// Геттеры для загруженных данных
const EnemyTemplate* DataStore::GetEnemyTemplate(int id) const {
  auto it = enemy_templates_.find(id);
  return (it != enemy_templates_.end()) ? &it->second : nullptr;
}

int DataStore::GetEnemyIdByName(const std::string& name) const {
  auto it = enemy_name_to_id_.find(name);
  return (it != enemy_name_to_id_.end()) ? it->second : -1;
}

const ScriptData* DataStore::GetScriptById(int id) const {
  auto it = scripts_.find(id);
  return (it != scripts_.end()) ? &it->second : nullptr;
}

int DataStore::GetScriptIdByName(const std::string& name) const {
  auto it = script_name_to_id_.find(name);
  return (it != script_name_to_id_.end()) ? it->second : -1;
}

std::vector<DialogueLine> DataStore::GetDialoguesForNpc(
    const std::string& npc_id, int memory, int fragments) const {
  std::vector<DialogueLine> result;
  for (const auto& [id, dlg] : dialogues_) {
    if (dlg.npc_id != npc_id) continue;
    if (memory < dlg.condition_memory_min || memory > dlg.condition_memory_max)
      continue;
    if (dlg.condition_fragments != -1 && fragments < dlg.condition_fragments)
      continue;
    result.push_back(dlg);
  }
  return result;
}

int DataStore::GetNpcDefaultDialogue(const std::string& npc_id) const {
  auto it = npc_default_dialogue_.find(npc_id);
  return (it != npc_default_dialogue_.end()) ? it->second : -1;
}

int DataStore::GetScriptIdByItemId(const std::string& item_id) const {
  auto it = item_to_script_id_.find(item_id);
  return (it != item_to_script_id_.end()) ? it->second : -1;
}

const std::vector<int>& DataStore::GetEnemyGroup(int location_id) const {
  static const std::vector<int> empty;
  auto it = enemy_groups_.find(location_id);
  return (it != enemy_groups_.end()) ? it->second : empty;
}

const std::vector<std::string>& DataStore::GetMemoryFragments() const {
  return memory_fragments_;
}

const PuzzleData* DataStore::GetPuzzleByLocation(int location_id) const {
  for (const auto& [id, puzzle] : puzzles_) {
    if (puzzle.location_id == location_id) return &puzzle;
  }
  return nullptr;
}

}  // namespace kernel
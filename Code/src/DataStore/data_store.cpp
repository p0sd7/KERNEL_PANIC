#include "data_store.h"

#include <algorithm>
#include <fstream>

#include "../Logging/logger.h"
#include "csv_loader.h"

namespace kernel {

void DataStore::LoadAll(const std::string& assets_path) {
  std::string config_path = assets_path + "interface_config.csv";
  if (!csv_loader::LoadInterfaceConfig(config_path, interface_config_)) {
    logging::LogWarning("Using default interface config");
  }

  std::string loc_path = assets_path + "locations.csv";
  if (!csv_loader::LoadLocations(loc_path, locations_)) {
    logging::LogError("Failed to load " + loc_path);
  }

  for (const auto& [id, loc] : locations_) {
    if (loc.ascii_background.empty()) continue;
    std::string bg_path = assets_path + "/" + loc.ascii_background;
    std::ifstream file(bg_path);
    if (!file.is_open()) {
      logging::LogError("Cannot open background file: " + bg_path);
      continue;
    }
    BackgroundData bg;
    std::string line;
    while (std::getline(file, line)) {
      if (!line.empty() && line.back() == '\r') line.pop_back();
      if (static_cast<int>(line.size()) < interface_config_.map_width) {
        line.append(interface_config_.map_width - line.size(), ' ');
      } else if (static_cast<int>(line.size()) > interface_config_.map_width) {
        line = line.substr(0, interface_config_.map_width);
      }
      bg.lines.push_back(line);
    }
    while (static_cast<int>(bg.lines.size()) < interface_config_.map_height) {
      bg.lines.emplace_back(interface_config_.map_width, ' ');
    }
    backgrounds_[id] = std::move(bg);
  }

  std::string map_path = assets_path + "map_objects.csv";
  if (!csv_loader::LoadMapObjects(map_path, map_objects_)) {
    logging::LogError("Failed to load " + map_path);
  }

  std::string npcs_path = assets_path + "npcs.csv";
  if (!csv_loader::LoadNpcs(npcs_path, npc_base_)) {
    logging::LogError("Failed to load " + npcs_path);
  }

  std::string dialogues_path = assets_path + "dialogues.csv";
  if (!csv_loader::LoadDialogues(dialogues_path, dialogues_)) {
    logging::LogError("Failed to load " + dialogues_path);
  }

  std::string items_path = assets_path + "items.csv";
  if (!csv_loader::LoadItems(items_path, items_)) {
    logging::LogError("Failed to load " + items_path);
  }

  std::string scripts_path = assets_path + "scripts.csv";
  if (!csv_loader::LoadScripts(scripts_path, scripts_)) {
    logging::LogError("Failed to load " + scripts_path);
  }

  std::string enemies_path = assets_path + "enemies.csv";
  if (!csv_loader::LoadEnemies(enemies_path, enemy_templates_)) {
    logging::LogError("Failed to load " + enemies_path);
  }

  std::string groups_path = assets_path + "enemy_groups.csv";
  if (!csv_loader::LoadEnemyGroups(groups_path, enemy_groups_)) {
    logging::LogError("Failed to load " + groups_path);
  }

  std::string memory_path = assets_path + "memory_fragments.csv";
  if (!csv_loader::LoadMemoryFragments(memory_path, memory_fragments_)) {
    logging::LogError("Failed to load " + memory_path);
  }

  std::string puzzles_path = assets_path + "puzzles.csv";
  if (!csv_loader::LoadPuzzles(puzzles_path, puzzles_)) {
    logging::LogError("Failed to load " + puzzles_path);
  }
  int map_w = interface_config_.map_width;
  int map_h = interface_config_.map_height;
  for (auto& [loc_id, vec] : map_objects_) {
    for (auto& obj : vec) {
      if (obj.x < 0 || obj.x >= map_w || obj.y < 0 || obj.y >= map_h) {
        logging::LogWarning("Map object " + std::to_string(obj.id) +
                            " out of bounds, corrected");
        obj.x = std::clamp(obj.x, 0, map_w - 1);
        obj.y = std::clamp(obj.y, 0, map_h - 1);
      }
    }
  }
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

Entity* DataStore::GetEntity(int index) const {
  if (index >= 0 && static_cast<size_t>(index) < entities_.size()) {
    return entities_[index].get();
  }
  return nullptr;
}

const LocationData* DataStore::GetLocationById(int id) const {
  auto it = locations_.find(id);
  return (it != locations_.end()) ? &it->second : nullptr;
}

const std::vector<MapObjectData>& DataStore::GetMapObjects(
    int location_id) const {
  static const std::vector<MapObjectData> empty;
  auto it = map_objects_.find(location_id);
  return (it != map_objects_.end()) ? it->second : empty;
}

const BackgroundData& DataStore::GetBackground(int location_id) const {
  static const BackgroundData empty;
  auto it = backgrounds_.find(location_id);
  return (it != backgrounds_.end()) ? it->second : empty;
}

const EnemyTemplate* DataStore::GetEnemyTemplate(int id) const {
  auto it = enemy_templates_.find(id);
  return (it != enemy_templates_.end()) ? &it->second : nullptr;
}

const ScriptData* DataStore::GetScriptById(int id) const {
  auto it = scripts_.find(id);
  return (it != scripts_.end()) ? &it->second : nullptr;
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

int DataStore::GetNpcDefaultDialogue(int npc_id) const {
  return npc_base_.at(npc_id).default_dialog_id;
}

const std::string& DataStore::GetNpcName(int npc_id) const {
  return npc_base_.at(npc_id).name;
}

const ItemData* DataStore::GetItemById(int id) const {
  auto it = items_.find(id);
  return (it != items_.end()) ? &it->second : nullptr;
}

const std::vector<int>& DataStore::GetEnemyGroup(int location_id) const {
  static const std::vector<int> empty;
  auto it = enemy_groups_.find(location_id);
  return (it != enemy_groups_.end()) ? it->second : empty;
}

const std::map<int, MemoryFragmentData>& DataStore::GetMemoryFragments() {
  return memory_fragments_;
}

const PuzzleData* DataStore::GetPuzzleByLocation(int location_id) const {
  for (const auto& [id, puzzle] : puzzles_) {
    if (puzzle.location_id == location_id) return &puzzle;
  }
  return nullptr;
}

void DataStore::RemoveMapObject(int location_id, int object_id) {
  auto it = map_objects_.find(location_id);
  if (it != map_objects_.end()) {
    auto& vec = it->second;
    vec.erase(std::remove_if(vec.begin(), vec.end(),
                             [object_id](const MapObjectData& obj) {
                               return obj.id == object_id;
                             }),
              vec.end());
  }
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

void DataStore::IncrementFragments() { ++fragments_collected_; }

void DataStore::ResetPlayerForNewCycle() {
  inventory_script_ids_.clear();
  memory_percent_ = 0;
  fragments_collected_ = 0;
  player_location_id_ = 1;
  if (player_location_id_ == -1) player_location_id_ = 1;
  Entity* player = GetEntity(player_index_);
  if (player) {
    player->SetStat(StatType::kHp, player->GetStat(StatType::kMaxHp));
    auto& objects = GetMapObjects(player_location_id_);
    for (MapObjectData obj : objects) {
      if (obj.type == "player") {
        player->SetPosition(obj.x, obj.y);
      }
    }
  }
}

int DataStore::GetLocationIdByName(std::string loc_name = "") const {
  int loc_id = -1;
  for (auto& loc : locations_) {
    if (loc.second.name == loc_name) {
      loc_id = loc.first;
    }
  }
  return loc_id;
}

}  // namespace kernel
#pragma once

#include <map>
#include <string>
#include <vector>

#include "data_store.h"

namespace kernel {
namespace csv_loader {

std::vector<std::string> parseLine(const std::string& line,
                                   char delimiter = ';');

bool loadInterfaceConfig(const std::string& path, InterfaceConfig& out_config);
bool loadLocations(const std::string& path,
                   std::map<int, LocationData>& out_locations);
bool loadMapObjects(const std::string& path,
                    std::map<int, std::vector<MapObjectData>>& out_map_objects);
bool loadNpcs(const std::string& path, std::map<int, NpcData>& out_npcs);
bool loadDialogues(const std::string& path,
                   std::map<int, DialogueLine>& out_dialogues);
bool loadItems(const std::string& path, std::map<int, ItemData>& out_items);
bool loadScripts(const std::string& path,
                 std::map<int, ScriptData>& out_scripts);
bool loadEnemies(const std::string& path,
                 std::map<int, EnemyTemplate>& out_enemies);
bool loadEnemyGroups(const std::string& path,
                     std::map<int, std::vector<EnemyGroup>>& out_groups);
bool loadMemoryFragments(const std::string& path,
                         std::map<int, MemoryFragmentData>& out_fragments);
bool loadPuzzles(const std::string& path,
                 std::map<int, PuzzleData>& out_puzzles);

}  // namespace csv_loader
}  // namespace kernel
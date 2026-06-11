#pragma once

#include <map>
#include <string>
#include <vector>

#include "data_store.h"

namespace kernel {
namespace csv_loader {

std::vector<std::string> ParseLine(const std::string& line,
                                   char delimiter = ';');

bool LoadInterfaceConfig(const std::string& path, InterfaceConfig& out_config);

bool LoadLocations(const std::string& path,
                   std::map<int, LocationData>& out_locations);

bool LoadMapObjects(const std::string& path,
                    std::map<int, std::vector<MapObjectData>>& out_map_objects);

bool LoadNpcs(const std::string& path,
              std::map<int, NpcData>& out_npc_default_dialogue);

bool LoadDialogues(const std::string& path,
                   std::map<int, DialogueLine>& out_dialogues);

bool LoadItems(const std::string& path, std::map<int, ItemData>& out_items);

bool LoadScripts(const std::string& path,
                 std::map<int, ScriptData>& out_scripts);

bool LoadEnemies(const std::string& path,
                 std::map<int, EnemyTemplate>& out_enemies);

bool LoadEnemyGroups(const std::string& path,
                     std::map<int, std::vector<int>>& out_groups);

bool LoadMemoryFragments(const std::string& path,
                         std::map<int, MemoryFragmentData>& out_fragments);

bool LoadPuzzles(const std::string& path,
                 std::map<int, PuzzleData>& out_puzzles);

}  // namespace csv_loader
}  // namespace kernel
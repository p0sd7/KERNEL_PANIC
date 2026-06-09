#include "csv_loader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "../Logging/logger.h"
#include "csv_columns.h"

namespace kernel {
namespace csv_loader {

std::string Trim(const std::string& s) {
  size_t start = s.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) return "";
  size_t end = s.find_last_not_of(" \t\r\n");
  return s.substr(start, end - start + 1);
}

std::vector<std::string> ParseLine(const std::string& line, char delimiter) {
  std::vector<std::string> result;
  std::string token;
  bool in_quotes = false;
  for (size_t i = 0; i < line.size(); ++i) {
    char ch = line[i];
    if (ch == '"') {
      in_quotes = !in_quotes;
      continue;
    }
    if (ch == delimiter && !in_quotes) {
      result.push_back(Trim(token));
      token.clear();
    } else {
      token += ch;
    }
  }
  result.push_back(Trim(token));
  return result;
}

static std::unordered_map<std::string, int> BuildColumnMap(
    const std::vector<std::string>& headers) {
  std::unordered_map<std::string, int> map;
  for (size_t i = 0; i < headers.size(); ++i) {
    map[headers[i]] = static_cast<int>(i);
  }
  return map;
}

static bool CheckRequiredColumns(
    const std::unordered_map<std::string, int>& col,
    const std::vector<const char*>& required, const std::string& path) {
  bool ok = true;
  for (const char* name : required) {
    if (col.find(name) == col.end()) {
      logging::LogError(std::string("Missing column '") + name + "' in " +
                        path);
      ok = false;
    }
  }
  return ok;
}

static int SafeStoi(const std::string& s, int default_value = -1,
                    const std::string& context = "") {
  if (s.empty()) {
    if (!context.empty()) {
      logging::LogWarning("Empty string in " + context + ", using default " +
                          std::to_string(default_value));
    }
    return default_value;
  }
  try {
    return std::stoi(s);
  } catch (const std::exception& e) {
    logging::LogError("Failed to convert '" + s + "' to int in " + context +
                      ": " + e.what());
    return default_value;
  }
}

bool LoadLocations(const std::string& path,
                   std::map<int, LocationData>& out_locations,
                   std::map<std::string, int>& out_name_to_id) {
  std::ifstream file(path);
  if (!file.is_open()) {
    logging::LogError("LoadLocations: cannot open file " + path);
    return false;
  }

  std::string header_line;
  std::getline(file, header_line);
  auto headers = ParseLine(header_line, ';');
  auto col = BuildColumnMap(headers);

  std::vector<const char*> required = {
      csv_column::kLocId,
      csv_column::kLocName,
      csv_column::kLocAsciiBackground,
      csv_column::kLocExitSymbol,
      csv_column::kLocForcedCombat,
      csv_column::kLocNextLocationId,
  };
  if (!CheckRequiredColumns(col, required, path)) return false;

  std::string line;
  int line_num = 1;
  while (std::getline(file, line)) {
    ++line_num;
    if (line.empty()) continue;
    auto cols = ParseLine(line, ';');
    if (cols.size() < headers.size()) {
      logging::LogWarning("Skipping line " + std::to_string(line_num) + " in " +
                          path + ": expected " +
                          std::to_string(headers.size()) + " columns, got " +
                          std::to_string(cols.size()));
      continue;
    }

    LocationData loc;
    loc.id = SafeStoi(cols[col[csv_column::kLocId]], -1,
                      path + " id line " + std::to_string(line_num));
    loc.name = cols[col[csv_column::kLocName]];
    loc.ascii_background = cols[col[csv_column::kLocAsciiBackground]];
    std::string exit_sym = cols[col[csv_column::kLocExitSymbol]];
    loc.exit_symbol = exit_sym.empty() ? 0 : exit_sym[0];
    loc.forced_combat_on_enter =
        (cols[col[csv_column::kLocForcedCombat]] == "1");
    std::string next_id = cols[col[csv_column::kLocNextLocationId]];
    loc.next_location_id = next_id.empty()
                               ? -1
                               : SafeStoi(next_id, -1,
                                          path + " next_location_id line " +
                                              std::to_string(line_num));

    out_locations[loc.id] = loc;
    out_name_to_id[loc.name] = loc.id;
  }
  logging::LogInfo("Loaded " + std::to_string(out_locations.size()) +
                   " locations from " + path);
  return true;
}

bool LoadMapObjects(
    const std::string& path,
    std::map<int, std::vector<MapObjectData>>& out_map_objects) {
  std::ifstream file(path);
  if (!file.is_open()) {
    logging::LogError("LoadMapObjects: cannot open file " + path);
    return false;
  }

  std::string header_line;
  std::getline(file, header_line);
  auto headers = ParseLine(header_line, ';');
  auto col = BuildColumnMap(headers);

  std::vector<const char*> required = {
      csv_column::kMapId,    csv_column::kMapLocationId,
      csv_column::kMapType,  csv_column::kMapSymbol,
      csv_column::kMapX,     csv_column::kMapY,
      csv_column::kMapRefId, csv_column::kMapSpecialCondition,
  };
  if (!CheckRequiredColumns(col, required, path)) return false;

  std::string line;
  int line_num = 1;
  while (std::getline(file, line)) {
    ++line_num;
    if (line.empty()) continue;
    auto cols = ParseLine(line, ';');
    if (cols.size() < headers.size()) {
      logging::LogWarning("Skipping line " + std::to_string(line_num) + " in " +
                          path + ": expected " +
                          std::to_string(headers.size()) + " columns, got " +
                          std::to_string(cols.size()));
      continue;
    }

    MapObjectData obj;
    obj.id = SafeStoi(cols[col[csv_column::kMapId]], -1,
                      path + " id line " + std::to_string(line_num));
    obj.location_id =
        SafeStoi(cols[col[csv_column::kMapLocationId]], -1,
                 path + " location_id line " + std::to_string(line_num));
    obj.type = cols[col[csv_column::kMapType]];
    std::string sym = cols[col[csv_column::kMapSymbol]];
    obj.symbol = sym.empty() ? '?' : sym[0];
    obj.x = SafeStoi(cols[col[csv_column::kMapX]], 0,
                     path + " x line " + std::to_string(line_num));
    obj.y = SafeStoi(cols[col[csv_column::kMapY]], 0,
                     path + " y line " + std::to_string(line_num));
    obj.ref_id = cols[col[csv_column::kMapRefId]];
    obj.special_condition = cols[col[csv_column::kMapSpecialCondition]];

    out_map_objects[obj.location_id].push_back(obj);
  }
  logging::LogInfo("Loaded map objects for " +
                   std::to_string(out_map_objects.size()) + " locations from " +
                   path);
  return true;
}

bool LoadNpcs(const std::string& path,
              std::map<std::string, int>& out_npc_default_dialogue) {
  std::ifstream file(path);
  if (!file.is_open()) {
    logging::LogError("LoadNpcs: cannot open file " + path);
    return false;
  }

  std::string header_line;
  std::getline(file, header_line);
  auto headers = ParseLine(header_line, ';');
  auto col = BuildColumnMap(headers);

  std::vector<const char*> required = {
      csv_column::kNpcId,
      csv_column::kNpcDefaultDialogue,
  };
  if (!CheckRequiredColumns(col, required, path)) return false;

  std::string line;
  int line_num = 1;
  while (std::getline(file, line)) {
    ++line_num;
    if (line.empty()) continue;
    auto cols = ParseLine(line, ';');
    if (cols.size() < headers.size()) {
      logging::LogWarning("Skipping line " + std::to_string(line_num) + " in " +
                          path + ": expected " +
                          std::to_string(headers.size()) + " columns, got " +
                          std::to_string(cols.size()));
      continue;
    }

    std::string npc_id = cols[col[csv_column::kNpcId]];
    int def_dlg =
        SafeStoi(cols[col[csv_column::kNpcDefaultDialogue]], -1,
                 path + " default_dialogue line " + std::to_string(line_num));
    out_npc_default_dialogue[npc_id] = def_dlg;
  }
  logging::LogInfo("Loaded " + std::to_string(out_npc_default_dialogue.size()) +
                   " NPCs from " + path);
  return true;
}

bool LoadDialogues(const std::string& path,
                   std::map<int, DialogueLine>& out_dialogues) {
  std::ifstream file(path);
  if (!file.is_open()) {
    logging::LogError("LoadDialogues: cannot open file " + path);
    return false;
  }

  std::string header_line;
  std::getline(file, header_line);
  auto headers = ParseLine(header_line, ';');
  auto col = BuildColumnMap(headers);

  std::vector<const char*> required = {
      csv_column::kDlgId,
      csv_column::kDlgNpcId,
      csv_column::kDlgCondMemoryMin,
      csv_column::kDlgCondMemoryMax,
      csv_column::kDlgCondFragments,
      csv_column::kDlgText,
  };
  if (!CheckRequiredColumns(col, required, path)) return false;

  std::string line;
  int line_num = 1;
  while (std::getline(file, line)) {
    ++line_num;
    if (line.empty()) continue;
    auto cols = ParseLine(line, ';');
    if (cols.size() < headers.size()) {
      logging::LogWarning("Skipping line " + std::to_string(line_num) + " in " +
                          path + ": expected " +
                          std::to_string(headers.size()) + " columns, got " +
                          std::to_string(cols.size()));
      continue;
    }

    DialogueLine dlg;
    dlg.id = SafeStoi(cols[col[csv_column::kDlgId]], -1,
                      path + " id line " + std::to_string(line_num));
    dlg.npc_id = cols[col[csv_column::kDlgNpcId]];
    dlg.condition_memory_min =
        SafeStoi(cols[col[csv_column::kDlgCondMemoryMin]], 0,
                 path + " memory_min line " + std::to_string(line_num));
    dlg.condition_memory_max =
        SafeStoi(cols[col[csv_column::kDlgCondMemoryMax]], 100,
                 path + " memory_max line " + std::to_string(line_num));
    std::string frag = cols[col[csv_column::kDlgCondFragments]];
    dlg.condition_fragments =
        frag.empty()
            ? -1
            : SafeStoi(frag, -1,
                       path + " fragments line " + std::to_string(line_num));
    dlg.text = cols[col[csv_column::kDlgText]];

    out_dialogues[dlg.id] = dlg;
  }
  logging::LogInfo("Loaded " + std::to_string(out_dialogues.size()) +
                   " dialogues from " + path);
  return true;
}

bool LoadItems(const std::string& path,
               std::map<std::string, int>& out_item_script_id) {
  std::ifstream file(path);
  if (!file.is_open()) {
    logging::LogError("LoadItems: cannot open file " + path);
    return false;
  }

  std::string header_line;
  std::getline(file, header_line);
  auto headers = ParseLine(header_line, ';');
  auto col = BuildColumnMap(headers);

  std::vector<const char*> required = {
      csv_column::kItemId,
      csv_column::kItemScriptId,
  };
  if (!CheckRequiredColumns(col, required, path)) return false;

  std::string line;
  int line_num = 1;
  while (std::getline(file, line)) {
    ++line_num;
    if (line.empty()) continue;
    auto cols = ParseLine(line, ';');
    if (cols.size() < headers.size()) {
      logging::LogWarning("Skipping line " + std::to_string(line_num) + " in " +
                          path + ": expected " +
                          std::to_string(headers.size()) + " columns, got " +
                          std::to_string(cols.size()));
      continue;
    }

    std::string item_id = cols[col[csv_column::kItemId]];
    std::string script_id_str = cols[col[csv_column::kItemScriptId]];
    int script_id =
        script_id_str.empty()
            ? -1
            : SafeStoi(script_id_str, -1,
                       path + " script_id line " + std::to_string(line_num));
    out_item_script_id[item_id] = script_id;
  }
  logging::LogInfo("Loaded " + std::to_string(out_item_script_id.size()) +
                   " items from " + path);
  return true;
}

bool LoadScripts(const std::string& path,
                 std::map<int, ScriptData>& out_scripts,
                 std::map<std::string, int>& out_script_name_to_id) {
  std::ifstream file(path);
  if (!file.is_open()) {
    logging::LogError("LoadScripts: cannot open file " + path);
    return false;
  }

  std::string header_line;
  std::getline(file, header_line);
  auto headers = ParseLine(header_line, ';');
  auto col = BuildColumnMap(headers);

  std::vector<const char*> required = {
      csv_column::kScriptId,
      csv_column::kScriptNameForInput,
      csv_column::kScriptDamage,
      csv_column::kScriptSelfDamage,
      csv_column::kScriptDefensePercent,
      csv_column::kScriptStunTarget,
      csv_column::kScriptAvailableAfterBoss,
  };
  if (!CheckRequiredColumns(col, required, path)) return false;

  std::string line;
  int line_num = 1;
  while (std::getline(file, line)) {
    ++line_num;
    if (line.empty()) continue;
    auto cols = ParseLine(line, ';');
    if (cols.size() < headers.size()) {
      logging::LogWarning("Skipping line " + std::to_string(line_num) + " in " +
                          path + ": expected " +
                          std::to_string(headers.size()) + " columns, got " +
                          std::to_string(cols.size()));
      continue;
    }

    ScriptData scr;
    scr.id = SafeStoi(cols[col[csv_column::kScriptId]], -1,
                      path + " id line " + std::to_string(line_num));
    scr.name_for_input = cols[col[csv_column::kScriptNameForInput]];
    scr.damage = SafeStoi(cols[col[csv_column::kScriptDamage]], 0,
                          path + " damage line " + std::to_string(line_num));
    scr.self_damage =
        SafeStoi(cols[col[csv_column::kScriptSelfDamage]], 0,
                 path + " self_damage line " + std::to_string(line_num));
    scr.defense_percent =
        SafeStoi(cols[col[csv_column::kScriptDefensePercent]], 0,
                 path + " defense line " + std::to_string(line_num));
    scr.stun_target = (cols[col[csv_column::kScriptStunTarget]] == "1");
    scr.available_after_boss = cols[col[csv_column::kScriptAvailableAfterBoss]];

    out_scripts[scr.id] = scr;
    out_script_name_to_id[scr.name_for_input] = scr.id;
  }
  logging::LogInfo("Loaded " + std::to_string(out_scripts.size()) +
                   " scripts from " + path);
  return true;
}

bool LoadEnemies(const std::string& path,
                 std::map<int, EnemyTemplate>& out_enemies,
                 std::map<std::string, int>& out_enemy_name_to_id) {
  std::ifstream file(path);
  if (!file.is_open()) {
    logging::LogError("LoadEnemies: cannot open file " + path);
    return false;
  }

  std::string header_line;
  std::getline(file, header_line);
  auto headers = ParseLine(header_line, ';');
  auto col = BuildColumnMap(headers);

  std::vector<const char*> required = {
      csv_column::kEnemyId,           csv_column::kEnemyName,
      csv_column::kEnemyAsciiDisplay, csv_column::kEnemyHp,
      csv_column::kEnemyDamage,       csv_column::kEnemyType,
      csv_column::kEnemySpecialAi,    csv_column::kEnemyDialogueOnSpawn,
  };
  if (!CheckRequiredColumns(col, required, path)) return false;

  std::string line;
  int line_num = 1;
  while (std::getline(file, line)) {
    ++line_num;
    if (line.empty()) continue;
    auto cols = ParseLine(line, ';');
    if (cols.size() < headers.size()) {
      logging::LogWarning("Skipping line " + std::to_string(line_num) + " in " +
                          path + ": expected " +
                          std::to_string(headers.size()) + " columns, got " +
                          std::to_string(cols.size()));
      continue;
    }

    EnemyTemplate enemy;
    enemy.id = SafeStoi(cols[col[csv_column::kEnemyId]], -1,
                        path + " id line " + std::to_string(line_num));
    enemy.name = cols[col[csv_column::kEnemyName]];
    enemy.ascii_display = cols[col[csv_column::kEnemyAsciiDisplay]];
    enemy.hp = SafeStoi(cols[col[csv_column::kEnemyHp]], 0,
                        path + " hp line " + std::to_string(line_num));
    enemy.damage = SafeStoi(cols[col[csv_column::kEnemyDamage]], 0,
                            path + " damage line " + std::to_string(line_num));
    std::string type_str = cols[col[csv_column::kEnemyType]];
    enemy.enemy_type =
        (type_str == "boss") ? EnemyType::kBoss : EnemyType::kRandom;
    enemy.special_ai = cols[col[csv_column::kEnemySpecialAi]];
    enemy.dialogue_on_spawn = cols[col[csv_column::kEnemyDialogueOnSpawn]];

    out_enemies[enemy.id] = enemy;
    out_enemy_name_to_id[enemy.name] = enemy.id;
  }
  logging::LogInfo("Loaded " + std::to_string(out_enemies.size()) +
                   " enemies from " + path);
  return true;
}

bool LoadEnemyGroups(const std::string& path,
                     std::map<int, std::vector<int>>& out_groups) {
  std::ifstream file(path);
  if (!file.is_open()) {
    logging::LogError("LoadEnemyGroups: cannot open file " + path);
    return false;
  }

  std::string header_line;
  std::getline(file, header_line);
  auto headers = ParseLine(header_line, ';');
  auto col = BuildColumnMap(headers);

  std::vector<const char*> required = {
      csv_column::kGroupLocationId,
      csv_column::kGroupEnemyId,
  };
  if (!CheckRequiredColumns(col, required, path)) return false;

  std::string line;
  int line_num = 1;
  while (std::getline(file, line)) {
    ++line_num;
    if (line.empty()) continue;
    auto cols = ParseLine(line, ';');
    if (cols.size() < headers.size()) {
      logging::LogWarning("Skipping line " + std::to_string(line_num) + " in " +
                          path + ": expected " +
                          std::to_string(headers.size()) + " columns, got " +
                          std::to_string(cols.size()));
      continue;
    }

    int loc_id =
        SafeStoi(cols[col[csv_column::kGroupLocationId]], -1,
                 path + " location_id line " + std::to_string(line_num));
    int enemy_id =
        SafeStoi(cols[col[csv_column::kGroupEnemyId]], -1,
                 path + " enemy_id line " + std::to_string(line_num));
    if (loc_id != -1 && enemy_id != -1) {
      out_groups[loc_id].push_back(enemy_id);
    } else {
      logging::LogWarning("Skipping line " + std::to_string(line_num) + " in " +
                          path + " due to invalid IDs");
    }
  }
  logging::LogInfo("Loaded enemy groups for " +
                   std::to_string(out_groups.size()) + " locations from " +
                   path);
  return true;
}

bool LoadMemoryFragments(const std::string& path,
                         std::vector<std::string>& out_fragments) {
  std::ifstream file(path);
  if (!file.is_open()) {
    logging::LogError("LoadMemoryFragments: cannot open file " + path);
    return false;
  }

  std::string header_line;
  std::getline(file, header_line);
  auto headers = ParseLine(header_line, ';');
  auto col = BuildColumnMap(headers);

  std::vector<const char*> required = {
      csv_column::kMemText,
  };
  if (!CheckRequiredColumns(col, required, path)) return false;

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty()) continue;
    auto cols = ParseLine(line, ';');
    if (cols.size() < headers.size()) {
      logging::LogWarning("Skipping line in " + path + ": not enough columns");
      continue;
    }
    out_fragments.push_back(cols[col[csv_column::kMemText]]);
  }
  logging::LogInfo("Loaded " + std::to_string(out_fragments.size()) +
                   " memory fragments from " + path);
  return true;
}

bool LoadPuzzles(const std::string& path,
                 std::map<int, PuzzleData>& out_puzzles) {
  std::ifstream file(path);
  if (!file.is_open()) {
    logging::LogError("LoadPuzzles: cannot open file " + path);
    return false;
  }

  std::string header_line;
  std::getline(file, header_line);
  auto headers = ParseLine(header_line, ';');
  auto col = BuildColumnMap(headers);

  std::vector<const char*> required = {
      csv_column::kPuzzleId,           csv_column::kPuzzleLocationId,
      csv_column::kPuzzleType,         csv_column::kPuzzleSolutionData,
      csv_column::kPuzzleRewardItemId, csv_column::kPuzzleWrongPenalty,
  };
  if (!CheckRequiredColumns(col, required, path)) return false;

  std::string line;
  int line_num = 1;
  while (std::getline(file, line)) {
    ++line_num;
    if (line.empty()) continue;
    auto cols = ParseLine(line, ';');
    if (cols.size() < headers.size()) {
      logging::LogWarning("Skipping line " + std::to_string(line_num) + " in " +
                          path + ": expected " +
                          std::to_string(headers.size()) + " columns, got " +
                          std::to_string(cols.size()));
      continue;
    }

    PuzzleData puzzle;
    puzzle.id = SafeStoi(cols[col[csv_column::kPuzzleId]], -1,
                         path + " id line " + std::to_string(line_num));
    puzzle.location_id =
        SafeStoi(cols[col[csv_column::kPuzzleLocationId]], -1,
                 path + " location_id line " + std::to_string(line_num));
    puzzle.type = cols[col[csv_column::kPuzzleType]];
    puzzle.solution_data = cols[col[csv_column::kPuzzleSolutionData]];
    puzzle.reward_item_id = cols[col[csv_column::kPuzzleRewardItemId]];
    puzzle.wrong_penalty = cols[col[csv_column::kPuzzleWrongPenalty]];

    out_puzzles[puzzle.id] = puzzle;
  }
  logging::LogInfo("Loaded " + std::to_string(out_puzzles.size()) +
                   " puzzles from " + path);
  return true;
}

}  // namespace csv_loader
}  // namespace kernel
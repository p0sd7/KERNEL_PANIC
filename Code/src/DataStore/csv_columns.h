#pragma once

namespace kernel {
namespace csv_column {

// locations.csv
constexpr const char* kLocId = "id";
constexpr const char* kLocName = "name";
constexpr const char* kLocAsciiBackground = "ascii_background";
constexpr const char* kLocForcedCombat = "forced_combat_on_enter";
constexpr const char* kLocNextLocationId = "next_location_id";

// map_objects.csv
constexpr const char* kObjId = "id";
constexpr const char* kObjLocationId = "location_id";
constexpr const char* kObjType = "type";
constexpr const char* kObjSymbol = "symbol";
constexpr const char* kObjX = "x";
constexpr const char* kObjY = "y";
constexpr const char* kObjRefId = "ref_id";
constexpr const char* kObjSpecialCondition = "special_condition";

// npcs.csv
constexpr const char* kNpcId = "id";
constexpr const char* kNpcName = "name";
constexpr const char* kNpcDefaultDialogue = "default_dialogue_id";

// dialogues.csv
constexpr const char* kDlgId = "id";
constexpr const char* kDlgNpcId = "npc_id";
constexpr const char* kDlgCondMemoryMin = "condition_memory_min";
constexpr const char* kDlgCondMemoryMax = "condition_memory_max";
constexpr const char* kDlgCondFragments = "condition_fragments";
constexpr const char* kDlgText = "text";

// items.csv
constexpr const char* kItemId = "id";
constexpr const char* kItemName = "name";
constexpr const char* kItemType = "type";
constexpr const char* kItemEffectValue = "effect_value";
constexpr const char* kItemScriptId = "script_id";

// scripts.csv
constexpr const char* kScriptId = "id";
constexpr const char* kScriptNameForInput = "name_for_input";
constexpr const char* kScriptDamage = "damage";
constexpr const char* kScriptSelfDamage = "self_damage";
constexpr const char* kScriptDefensePercent = "defense_percent";
constexpr const char* kScriptStunTarget = "stun_target";
constexpr const char* kScriptAvailableAfterBoss = "available_after_boss";

// enemies.csv
constexpr const char* kEnemyId = "id";
constexpr const char* kEnemyName = "name";
constexpr const char* kEnemyAsciiDisplay = "ascii_display";
constexpr const char* kEnemyHp = "hp";
constexpr const char* kEnemyDamage = "damage";
constexpr const char* kEnemyType = "enemy_type";
constexpr const char* kEnemySpecialAi = "special_ai";
constexpr const char* kEnemyDialogueOnSpawn = "dialogue_on_spawn";

// enemy_groups.csv
constexpr const char* kGroupId = "id";
constexpr const char* kGroupLocationId = "location_id";
constexpr const char* kGroupEnemyId = "enemy_id";
constexpr const char* kGroupMinCount = "min_count";
constexpr const char* kGroupMaxCount = "max_count";
constexpr const char* kGroupSpawnChance = "spawn_chance";

// memory_fragments.csv
constexpr const char* kMemId = "id";
constexpr const char* kMemText = "text_fragment";

// puzzles.csv
constexpr const char* kPuzzleId = "id";
constexpr const char* kPuzzleLocationId = "location_id";
constexpr const char* kPuzzleType = "type";
constexpr const char* kPuzzleSolutionData = "solution_data";
constexpr const char* kPuzzleRewardItemId = "reward_item_id";
constexpr const char* kPuzzleWrongPenalty = "wrong_penalty";

// graphic interface_config.csv
constexpr const char* kInterfaceCfgKey = "key";
constexpr const char* kInterfaceCfgValue = "value";

}  // namespace csv_column
}  // namespace kernel
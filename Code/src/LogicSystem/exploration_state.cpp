// exploration_state.cpp
#include "exploration_state.h"

#include "../DataStore/data_store.h"
#include "../Logging/logger.h"
#include "../RenderSystem/console_renderer.h"
#include "combat_state.h"
#include "dialogue_state.h"
#include "final_state.h"
#include "game.h"
#include "puzzle_state.h"

namespace kernel {

ExplorationState::ExplorationState() = default;

void ExplorationState::handleInput(const InputCommand& cmd, DataStore& data,
                                   ConsoleRenderer& renderer) {
  if (cmd.type == InputType::kHelp) {
    static bool help_loaded = false;
    if (!help_loaded) {
      renderer.loadHelpText("../assets/help.txt");
      help_loaded = true;
    }
    renderer.toggleHelp();
    return;
  }

  if (renderer.isHelpVisible()) {
    if (cmd.type == InputType::kQuit) {
      if (game_) game_->quit();
    }
    return;
  }

  if (cmd.type == InputType::kMove) {
    int player_idx = data.getPlayer().entity_index;
    Entity* player = data.getEntity(player_idx);
    if (!player) return;

    int new_x = player->position().x + cmd.dx;
    int new_y = player->position().y + cmd.dy;

    int loc_id = data.getPlayer().location_id;
    const auto* loc = data.getLocationById(loc_id);
    int next_loc_id = loc->next_location_id;
    const auto& objects = data.getMapObjects(loc_id);

    const MapObjectData* target_obj = nullptr;
    for (const auto& obj : objects) {
      if (obj.x == new_x && obj.y == new_y) {
        target_obj = &obj;
        break;
      }
    }

    if (target_obj) {
      if (target_obj->type == "player") {
        data.removeMapObject(target_obj->location_id, target_obj->id);
      } else if (target_obj->type == "item") {
        const auto* item = data.getItemById(target_obj->ref_id);
        if (!item) {
          renderer.setTemporaryDialogue("System", {"Unknown item"});
          return;
        }
        int current_hp = player->getStat(StatType::kHp);
        int max_hp = player->getStat(StatType::kMaxHp);
        switch (item->type) {
          case ItemType::kHeal:
            player->setStat(StatType::kHp,
                            std::min(max_hp, current_hp + item->effect_value));
            break;
          case ItemType::kScript:
            if (item->script_id != -1)
              data.addScriptToInventory(item->script_id);
            break;
          case ItemType::kTrap:
            player->setStat(StatType::kHp, current_hp - item->effect_value);
            renderer.setTemporaryDialogue("LNK2019", {"hurts, isn't it?"});
            break;
          case ItemType::kMemoryFrag: {
            data.incrementFragments();
            int frag_id = item->effect_value;
            const auto& fragments = data.getMemoryFragments();
            auto it = fragments.find(frag_id);
            std::string msg = (it != fragments.end())
                                  ? it->second.text
                                  : "Unknown memory fragment";
            renderer.setTemporaryDialogue("memory fragment", {msg});
            break;
          }
          default:
            return;
        }
        data.removeMapObject(loc_id, target_obj->id);
      } else if (target_obj->type == "npc") {
        game_->pushState(
            std::make_unique<DialogueState>(data, target_obj->ref_id));
      } else if (target_obj->type == "boss") {
        if (next_loc_id != -1 && !data.getInventoryScripts().empty()) {
          data.getPlayer().location_id = next_loc_id;
          auto spawn = data.getSpawnPoint(next_loc_id);
          player->setPosition(spawn.first, spawn.second);
          game_->pushState(std::make_unique<CombatState>(
              data, target_obj->ref_id, target_obj->symbol));
        }
        return;
      } else if (target_obj->type == "exit") {
        if (next_loc_id != -1 && !data.getInventoryScripts().empty()) {
          data.getPlayer().location_id = next_loc_id;
          auto spawn = data.getSpawnPoint(next_loc_id);
          player->setPosition(spawn.first, spawn.second);
          const auto* new_loc = data.getLocationById(next_loc_id);
          if (new_loc && new_loc->forced_combat_on_enter) {
            game_->pushState(std::make_unique<CombatState>(data, next_loc_id));
          }
        } else {
          renderer.setTemporaryDialogue("System", {"No exit."});
        }
        return;
      } else if (target_obj->type == "puzzle") {
        game_->pushState(std::make_unique<PuzzleState>(data, loc_id));
        return;
      }
      return;
    }
    int max_x = data.getInterfaceConfig().map_width - 1;
    int max_y = data.getInterfaceConfig().map_height - 1;
    if (new_x > 0 && new_x < max_x && new_y > 0 && new_y < max_y) {
      player->setPosition(new_x, new_y);
    }
  } else if (cmd.type == InputType::kQuit) {
    if (game_) game_->quit();
  } else if (cmd.type == InputType::kHelp) {
    static bool help_loaded = false;
    if (!help_loaded) {
      renderer.loadHelpText("../assets/help.txt");
      help_loaded = true;
    }
    renderer.toggleHelp();
  }
}

void ExplorationState::update(float /*delta*/, DataStore& /*data*/) {}

void ExplorationState::draw(const DataStore& data, ConsoleRenderer& renderer) {
  renderer.drawExploration(data, data.getPlayer().entity_index);
}

}  // namespace kernel
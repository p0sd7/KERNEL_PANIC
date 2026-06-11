#include "exploration_state.h"

#include "../DataStore/data_store.h"
#include "../Logging/logger.h"
#include "../RenderSystem/console_renderer.h"
#include "game.h"

namespace kernel {

ExplorationState::ExplorationState() = default;

void ExplorationState::HandleInput(const InputCommand& cmd, DataStore& data) {
  if (cmd.type == InputType::kMove) {
    int player_idx = data.GetPlayerIndex();
    Entity* player = data.GetEntity(player_idx);
    if (!player) return;

    int new_x = player->Position().x + cmd.dx;
    int new_y = player->Position().y + cmd.dy;

    int loc_id = data.GetPlayerLocationId();
    const auto* loc = data.GetLocationById(loc_id);
    const auto& objects = data.GetMapObjects(loc_id);

    const MapObjectData* target_obj = nullptr;
    for (const auto& obj : objects) {
      if (obj.x == new_x && obj.y == new_y) {
        target_obj = &obj;
        break;
      }
    }

    if (target_obj) {
      if (target_obj->type == "player") {
        data.RemoveMapObject(target_obj->location_id, target_obj->id);
      } else if (target_obj->type == "item") {
        const auto* item = data.GetItemById(target_obj->ref_id);
        if (!item) {
          logging::LogError("Unknown item id: " +
                            std::to_string(target_obj->ref_id));
          RenderSystem::SetDialogText("System", {"Unkown item"});
          return;
        }
        int current_hp = player->GetStat(StatType::kHp);
        int max_hp = player->GetStat(StatType::kMaxHp);
        switch (item->type) {
          case ItemType::kHeal:
            player->SetStat(StatType::kHp,
                            std::min(max_hp, current_hp + item->effect_value));
            break;
          case ItemType::kScript:
            if (item->script_id != -1) {
              data.AddScriptToInventory(item->script_id);
              const auto* scr = data.GetScriptById(item->script_id);
              std::string scr_name = scr ? scr->name_for_input : "unknown";
            }
            break;
          case ItemType::kTrap:
            player->SetStat(StatType::kHp, current_hp - item->effect_value);
            break;
          case ItemType::kPuzzleItem:
            break;
          case ItemType::kMemoryFrag:
            data.IncrementFragments();
            // надо добавить в диалог добавление текста через dialogstate
            break;
          default:
            return;
        }
        data.RemoveMapObject(loc_id, target_obj->id);
      } else if (target_obj->type == "npc") {
        RenderSystem::SetDialogText(target_obj->ref_id == -1
                                        ? "NPC"
                                        : data.GetNpcName(target_obj->ref_id),
                                    {"I have nothing to tell"});
        // надо добавить в диалог добавление текста через dialogstate
      } else if (target_obj->type == "boss") {
        RenderSystem::SetDialogText("Boss", {"im da boss"});
        // надо реализовать combat state
      } else if (target_obj->type == "exit") {
        int next_loc_id = loc->next_location_id;
        const auto* next_loc = data.GetLocationById(next_loc_id);
        data.SetPlayerLocationId(loc->next_location_id);
        const auto& next_loc_objects = data.GetMapObjects(next_loc_id);
        for (const auto& obj : next_loc_objects) {
          if (obj.type == "player") {
            player->SetPosition(obj.x, obj.y);
          }
        }
      }
      return;
    }
    if (new_x < 39 && new_x > 0 && new_y < 14 && new_y > 0) {
      player->SetPosition(new_x, new_y);
    }
  } else if (cmd.type == InputType::kQuit) {
    if (game_) game_->Quit();
  }
}

void ExplorationState::Update(float /*delta*/, DataStore& /*data*/) {}

void ExplorationState::Draw(const DataStore& data) {
  int player_idx = data.GetPlayerIndex();
  RenderSystem::DrawExploration(data, player_idx);
}

}  // namespace kernel
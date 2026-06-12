#include "exploration_state.h"

#include "../DataStore/data_store.h"
#include "../Logging/logger.h"
#include "../RenderSystem/console_renderer.h"
#include "dialogue_state.h"
#include "game.h"

namespace kernel {

ExplorationState::ExplorationState() = default;

void ExplorationState::HandleInput(const InputCommand& cmd, DataStore& data) {
  if (cmd.type == InputType::kMove) {
    int player_idx = data.GetPlayer().entity_index;
    Entity* player = data.GetEntity(player_idx);
    if (!player) return;

    int new_x = player->Position().x + cmd.dx;
    int new_y = player->Position().y + cmd.dy;

    int loc_id = data.GetPlayer().location_id;
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
          RenderSystem::SetDialogueText("System", {"Unknown item"});
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
            if (item->script_id != -1)
              data.AddScriptToInventory(item->script_id);
            break;
          case ItemType::kTrap:
            player->SetStat(StatType::kHp, current_hp - item->effect_value);
            RenderSystem::SetDialogueText(
                "Trap", {"You trigger a trap and lose " +
                         std::to_string(item->effect_value) + " HP."});
            break;
          case ItemType::kPuzzleItem:
            break;
          case ItemType::kMemoryFrag: {
            data.IncrementFragments();
            int frag_id = item->effect_value;
            const auto& fragments = data.GetMemoryFragments();
            auto it = fragments.find(frag_id);
            std::string msg = (it != fragments.end())
                                  ? it->second.text
                                  : "Unknown memory fragment";
            RenderSystem::SetDialogueText("memory fragment", {msg});
            break;
          }
          default:
            return;
        }
        data.RemoveMapObject(loc_id, target_obj->id);
      } else if (target_obj->type == "npc") {
        game_->PushState(
            std::make_unique<DialogueState>(data, target_obj->ref_id));
      } else if (target_obj->type == "boss") {
        RenderSystem::SetDialogueText("Boss", {"I'm the boss!"});
      } else if (target_obj->type == "exit") {
        int next_loc_id = loc->next_location_id;
        if (next_loc_id != -1) {
          data.GetPlayer().location_id = next_loc_id;
          auto spawn = data.GetSpawnPoint(next_loc_id);
          player->SetPosition(spawn.first, spawn.second);
        } else {
          RenderSystem::SetDialogueText("System", {"No exit."});
        }
      }
      return;
    }
    int max_x = data.GetInterfaceConfig().map_width - 1;
    int max_y = data.GetInterfaceConfig().map_height - 1;
    if (new_x > 0 && new_x < max_x && new_y > 0 && new_y < max_y) {
      player->SetPosition(new_x, new_y);
    }
  } else if (cmd.type == InputType::kQuit) {
    if (game_) game_->Quit();
  }
}

void ExplorationState::Update(float /*delta*/, DataStore& /*data*/) {}

void ExplorationState::Draw(const DataStore& data) {
  int player_idx = data.GetPlayer().entity_index;
  RenderSystem::DrawExploration(data, player_idx);
}

}  // namespace kernel
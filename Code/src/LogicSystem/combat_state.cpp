#include "combat_state.h"

#include <chrono>
#include <random>
#include <thread>

#include "../DataStore/data_store.h"
#include "../InputSystem/input_handler.h"
#include "../Logging/logger.h"
#include "../Random/random_utils.h"
#include "../RenderSystem/console_renderer.h"
#include "final_state.h"
#include "game.h"
#include "gameover_state.h"

namespace kernel {

CombatState::CombatState(DataStore& data, int enemy_id, char symbol)
    : is_boss_fight_(true) {
  const auto* templ = data.getEnemyTemplate(enemy_id);
  if (!templ) {
    logging::logError("Unknown enemy id: " + std::to_string(enemy_id));
    combat_over_ = true;
    return;
  }
  auto enemy = std::make_unique<Entity>(templ->id, EntityType::kEnemy, symbol);
  enemy->setStat(StatType::kHp, templ->hp);
  enemy->setStat(StatType::kMaxHp, templ->hp);
  enemy->setStat(StatType::kDamage, templ->damage);
  enemy->setStat(StatType::kEnemyId, templ->id);
  int idx = data.addEntity(std::move(enemy));
  enemy_indices_.push_back(idx);
  combat_engine_.startCombat(data, data.getPlayer().entity_index,
                             enemy_indices_, boss_id_, boss_name_,
                             spawn_dialogue_shown_);
}

CombatState::CombatState(DataStore& data, int location_id) {
  const auto& group_entries = data.getEnemyGroup(location_id);
  if (group_entries.empty()) {
    logging::logError("No enemies for group at location " +
                      std::to_string(location_id));
    combat_over_ = true;
    return;
  }
  static std::mt19937 rng(std::random_device{}());
  for (const auto& entry : group_entries) {
    int count = entry.min_count;
    if (entry.max_count > entry.min_count) {
      count += random_utils::randomInt(0, entry.max_count - entry.min_count);
    }
    for (int i = 0; i < count; ++i) {
      const auto* templ = data.getEnemyTemplate(entry.enemy_id);
      if (!templ) continue;
      auto enemy = std::make_unique<Entity>(templ->id, EntityType::kEnemy, '?');
      enemy->setStat(StatType::kHp, templ->hp);
      enemy->setStat(StatType::kMaxHp, templ->hp);
      enemy->setStat(StatType::kDamage, templ->damage);
      enemy->setStat(StatType::kEnemyId, templ->id);
      enemy_indices_.push_back(data.addEntity(std::move(enemy)));
    }
  }
  if (!enemy_indices_.empty()) {
    combat_engine_.startCombat(data, data.getPlayer().entity_index,
                               enemy_indices_, boss_id_, boss_name_,
                               spawn_dialogue_shown_);
  }
}

void CombatState::applyTurnResult(DataStore& data) {
  Entity* player = data.getEntity(data.getPlayer().entity_index);
  if (combat_engine_.isCombatOver(enemy_indices_, data)) {
    combat_over_ = true;
    int heal = combat_engine_.getHealReward(enemy_indices_, data);
    if (is_boss_fight_) {
      data.setMemoryPercent(data.getMemoryPercent() + 30);
      for (const auto& [id, scr] : data.getScripts()) {
        if (scr.available_after_boss == std::to_string(boss_id_) &&
            !data.hasScriptInInventory(scr.id)) {
          data.addScriptToInventory(scr.id);
        }
      }
    }
    if (player) {
      int new_hp = std::min(player->getStat(StatType::kMaxHp),
                            player->getStat(StatType::kHp) + heal);
      player->setStat(StatType::kHp, new_hp);
    }
    data.removeEntities(enemy_indices_);
    if (data.getPlayer().location_id != 8) {
      game_->popState();
    } else {
      game_->changeState(std::make_unique<FinalState>());
    }
  } else {
    std::string enemy_log;
    combat_engine_.enemyTurn(data, data.getPlayer().entity_index,
                             enemy_indices_, player_defense_percent_,
                             enemy_log);
    player_defense_percent_ = 0;
    if (player && player->getStat(StatType::kHp) <= 0) {
      combat_over_ = true;
      game_->changeState(std::make_unique<GameOverState>());
      return;
    }
    if (combat_engine_.isCombatOver(enemy_indices_, data)) {
      applyTurnResult(data);
    }
  }
}

void CombatState::handleInput(const InputCommand& cmd, DataStore& data,
                              ConsoleRenderer& renderer) {
  if (combat_over_) return;

  if (cmd.type == InputType::kQuit) {
    if (game_) game_->quit();
    return;
  }
  if (cmd.type == InputType::kConfirm) {
    renderer.flushInput();
    int row, col;
    renderer.getInputPosition(row, col);
    renderer.setCursorPosition(row, col + 2);
    std::string input = InputSystem::readString();
    if (input.empty()) return;

    std::string script_name = input;
    int target_num = 1;
    size_t space_pos = input.find(' ');
    if (space_pos != std::string::npos) {
      script_name = input.substr(0, space_pos);
      try {
        target_num = std::stoi(input.substr(space_pos + 1));
        if (target_num < 1 ||
            target_num > static_cast<int>(enemy_indices_.size())) {
          applyTurnResult(data);
          return;
        }
      } catch (...) {
        applyTurnResult(data);
        return;
      }
    }

    int script_id = data.getScriptIdByName(script_name);
    if (script_id == -1) {
      applyTurnResult(data);
      return;
    }

    int target_index = target_num - 1;
    std::string log;
    bool success = combat_engine_.applyScript(
        data, data.getPlayer().entity_index, script_id, enemy_indices_,
        target_index, player_defense_percent_, log);
    if (!success) {
      applyTurnResult(data);
      return;
    }

    highlight_enemy_ = target_index;
    renderer.drawCombat(data, data.getPlayer().entity_index, enemy_indices_, "",
                        highlight_enemy_, is_boss_fight_, boss_id_);
    renderer.present();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    highlight_enemy_ = -1;

    applyTurnResult(data);
  }
}

void CombatState::update(float /*delta*/, DataStore& /*data*/) {}

void CombatState::draw(const DataStore& data, ConsoleRenderer& renderer) {
  if (spawn_dialogue_shown_) {
    if (is_boss_fight_ && boss_id_ != -1) {
      const auto* templ = data.getEnemyTemplate(boss_id_);
      if (templ) {
        renderer.setCombatDialogue(boss_name_, {templ->dialogue_on_spawn});
      }
    } else if (!enemy_indices_.empty()) {
      const Entity* first_enemy = data.getEntity(enemy_indices_[0]);
      if (first_enemy) {
        int enemy_id = first_enemy->getStat(StatType::kEnemyId);
        const auto* templ = data.getEnemyTemplate(enemy_id);
        if (templ && !templ->dialogue_on_spawn.empty()) {
          renderer.setCombatDialogue(templ->name, {templ->dialogue_on_spawn});
        }
      }
    }
    spawn_dialogue_shown_ = false;
  }

  renderer.drawCombat(data, data.getPlayer().entity_index, enemy_indices_,
                      combat_log_, highlight_enemy_, is_boss_fight_, boss_id_);
}

}  // namespace kernel
#pragma once

#include <array>

#include "renderable.h"

namespace kernel {

enum class EntityType { kPlayer, kNpc, kItem, kEnemy, kTrap, kExit };

enum class StatType {
  kHp,
  kMaxHp,
  kDamage,
  kDefense,
  kScriptId,
  kHealValue,
  kNpcId,
  kEnemyId,
  kTrapDamage,
  kCount  // must be last
};

class Entity : public Renderable {
 public:
  Entity(int id, EntityType type, char symbol, int x, int y);
  Entity(int id, EntityType type, char symbol);

  char symbol() const override { return symbol_; }
  Pair position() const override { return {x_, y_}; }

  void setPosition(int x, int y);
  int getStat(StatType type) const;
  void setStat(StatType type, int value);
  EntityType getType() const { return type_; }
  int getId() const { return id_; }

 private:
  int id_;
  EntityType type_;
  int x_ = 0;
  int y_ = 0;
  char symbol_;
  std::array<int, static_cast<size_t>(StatType::kCount)> stats_ = {};
};

}  // namespace kernel
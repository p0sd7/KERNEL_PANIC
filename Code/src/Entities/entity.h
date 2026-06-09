#pragma once

#include <string>
#include <unordered_map>

#include "renderable.h"

namespace kernel {

enum class EntityType {
  kPlayer,
  kNpc,
  kItem,
  kBoss,
  kTrap,
  kExit,
};

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
  kCount  // Must be last
};

class Entity : public Renderable {
 public:
  Entity(int id, EntityType type, char symbol, int x, int y);

  char Symbol() const override { return symbol_; }
  Pair Position() const override { return {x_, y_}; }

  void SetPosition(int x, int y) {
    x_ = x;
    y_ = y;
  }
  int GetStat(const std::string& key, int default_value = 0) const;
  void SetStat(const std::string& key, int value);

  EntityType GetType() const { return type_; }
  int GetId() const { return id_; }

 private:
  int id_;
  EntityType type_;
  int x_;
  int y_;
  char symbol_;
  std::unordered_map<std::string, int> stats_;
};

} 
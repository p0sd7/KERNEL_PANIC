#include "entity.h"

namespace kernel {

Entity::Entity(int id, EntityType type, char symbol, int x, int y)
    : id_(id), type_(type), symbol_(symbol), x_(x), y_(y) {}

int Entity::GetStat(StatType type) const {
  return stats_[static_cast<size_t>(type)];
}

void Entity::SetStat(StatType type, int value) {
  stats_[static_cast<size_t>(type)] = value;
}

}
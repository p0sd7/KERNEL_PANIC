#include "entity.h"

namespace kernel {

Entity::Entity(int id, EntityType type, char symbol, int x, int y)
    : id_(id), type_(type), symbol_(symbol), x_(x), y_(y) {}

Entity::Entity(int id, EntityType type, char symbol)
    : id_(id), type_(type), symbol_(symbol) {}

void Entity::setPosition(int x, int y) {
  x_ = x;
  y_ = y;
}

int Entity::getStat(StatType type) const {
  return stats_[static_cast<size_t>(type)];
}

void Entity::setStat(StatType type, int value) {
  stats_[static_cast<size_t>(type)] = value;
}

}  // namespace kernel
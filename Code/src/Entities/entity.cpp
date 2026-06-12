#include "entity.h"

namespace kernel {

Entity::Entity(int id, EntityType type, char symbol, int x, int y)
    : id_(id), type_(type), symbol_(symbol), x_(x), y_(y) {}
Entity::Entity(int id, EntityType type, char symbol)
    : id_(id), type_(type), symbol_(symbol) {}
}  // namespace kernel
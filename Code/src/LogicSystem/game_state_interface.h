#pragma once

#include "../InputSystem/input_command.h"

namespace kernel {

class DataStore;
class Game;
class ConsoleRenderer;

class GameStateInterface {
 public:
  virtual ~GameStateInterface() = default;
  virtual void handleInput(const InputCommand& cmd, DataStore& data,
                           ConsoleRenderer& renderer) = 0;
  virtual void update(float delta, DataStore& data) = 0;
  virtual void draw(const DataStore& data, ConsoleRenderer& renderer) = 0;
  virtual void setContext(Game* game) = 0;
};

}  // namespace kernel
#pragma once

#include "../InputSystem/input_command.h"

namespace kernel {

class DataStore;
class Game;

class GameStateInterface {
 public:
  virtual ~GameStateInterface() = default;
  virtual void HandleInput(const InputCommand& cmd, DataStore& data) = 0;
  virtual void Update(float delta, DataStore& data) = 0;
  virtual void Draw(const DataStore& data) = 0;
  virtual void SetContext(Game* game) = 0;
};

}  // namespace kernel
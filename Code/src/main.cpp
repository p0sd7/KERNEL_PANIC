#include "Logging/logger.h"
#include "LogicSystem/game.h"

int main() {
  kernel::logging::init("logs/kernel_panic.log");
  kernel::Game game;
  game.run();
  kernel::logging::shutdown();
  return 0;
}
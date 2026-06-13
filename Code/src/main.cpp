#include "Logging/logger.h"
#include "LogicSystem/game.h"
#include "RenderSystem/console_renderer.h"

int main() {
  kernel::logging::Init("logs/kernel_panic.log");
  srand(time(nullptr));
  kernel::Game game;
  game.Run();
  kernel::logging::Shutdown();
  return 0;
}
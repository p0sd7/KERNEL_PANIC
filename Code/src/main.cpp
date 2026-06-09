#include <iostream>

#include "DataStore/data_store.h"
#include "Logging/logger.h"

int main() {
  kernel::logging::Init("logs/kernel_panic.log");
  kernel::DataStore data;
  data.LoadAll("../assets");

  for (int i = 1; i <= 7; ++i) {
    const auto* script = data.GetScriptById(i);
    if (script) {
      std::cout << "OK: id=" << script->id << " name=" << script->name_for_input
                << " damage=" << script->damage
                << " self_damage=" << script->self_damage
                << " defense=" << script->defense_percent
                << " stun=" << script->stun_target
                << " boss=" << script->available_after_boss << "\n";
    } else {
      std::cout << "FAIL: script id " << i << " not found\n";
    }
  }

  return 0;
}
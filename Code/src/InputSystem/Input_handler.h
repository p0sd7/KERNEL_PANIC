#pragma once

#include "input_command.h"

namespace kernel {
namespace InputSystem {

InputCommand pollEvents();
std::string readString();

}  // namespace InputSystem
}  // namespace kernel
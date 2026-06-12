#pragma once

#include "input_command.h"

namespace kernel {
namespace InputSystem {

InputCommand PollEvents();
std::string ReadString();

}  // namespace InputSystem
}  // namespace kernel
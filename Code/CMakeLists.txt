cmake_minimum_required(VERSION 3.16)
project(KERNEL_PANIC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Curses REQUIRED)

set(SOURCES
    src/main.cpp
    src/CombatSystem/combat_engine.cpp
    src/Entities/entity.cpp
    src/InputSystem/input_handler.cpp
    src/LogicSystem/combat_state.cpp
    src/LogicSystem/dialogue_state.cpp
    src/LogicSystem/exploration_state.cpp
    src/LogicSystem/final_state.cpp
    src/LogicSystem/game.cpp
    src/LogicSystem/gameover_state.cpp
    src/LogicSystem/puzzle_state.cpp
    src/RenderSystem/console_renderer.cpp
    src/DataStore/data_store.cpp
    src/DataStore/csv_loader.cpp
    src/Logging/logger.cpp
    src/Random/random_utils.cpp
)

add_executable(kernel_panic ${SOURCES})

target_link_libraries(kernel_panic ${CURSES_LIBRARIES})

target_include_directories(kernel_panic PRIVATE ${CURSES_INCLUDE_DIRS})
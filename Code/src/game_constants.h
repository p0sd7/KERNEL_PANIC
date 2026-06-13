#pragma once

namespace kernel {
namespace game_constants {

inline constexpr int kDefaultPlayerHp = 100;
inline constexpr int kDefaultPlayerMaxHp = 100;

inline constexpr int kStartLocationId = 1;
inline constexpr int kFinalLocationId = 8;

inline constexpr int kMemoryRewardForBoss = 30;
inline constexpr int kHealAfterBoss = 30;
inline constexpr int kHealAfterNormal = 5;

inline constexpr int kStunTurns = 2;

inline constexpr int kHealerMemoryBonus = 5;
inline constexpr int kHealerScriptId = 6;

}  // namespace game_constants
}  // namespace kernel
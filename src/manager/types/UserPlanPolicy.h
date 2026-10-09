#pragma once

namespace pendarlab::app::mavlink_hub
{
  /// How plan validation failures affect execution (see IManager::executePlan).
  enum class UserPlanPolicy {
    DISCARD,    ///< If any plan entry is invalid, execute nothing.
    BEST_EFFORT ///< Apply the valid entries and skip/report the invalid ones.
  };
} // namespace pendarlab::app::mavlink_hub

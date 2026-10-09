#pragma once

#include <string>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  /// Aggregated result of an operation: a success flag plus human-readable messages.
  struct OperationResult {
    bool success = true;               ///< Whether the operation succeeded overall.
    std::vector<std::string> messages; ///< Diagnostic/report messages produced by the operation.

    /// Combine \p other into this result.
    ///
    /// This result is successful only if it was already successful and \p other is too;
    /// \p other's messages are appended. Used to aggregate multiple sub-operation results.
    inline void merge(const OperationResult& other)
    {
      success = success && other.success;
      messages.insert(messages.end(), other.messages.begin(), other.messages.end());
    }
  };
} // namespace pendarlab::app::mavlink_hub

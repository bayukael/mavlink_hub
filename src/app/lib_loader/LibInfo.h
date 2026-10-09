#pragma once

#include <string>

namespace pendarlab::app::mavlink_hub
{
  /// Identifies a dynamically-loaded plugin library and the symbol it exports.
  struct LibInfo {
    std::string name; ///< Registration name the loaded definition is stored under.
    std::string path; ///< Filesystem path of the shared library (.so).
    std::string sym;  ///< Name of the exported symbol to resolve (a definition getter).
  };
} // namespace pendarlab::app::mavlink_hub
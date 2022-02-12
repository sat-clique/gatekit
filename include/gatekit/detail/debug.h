#pragma once

#define GATEKIT_DEBUG_WARNING                                                                      \
  "gatekit/debug.h has been included. This header should only be included for debugging."

#if defined(__GNUG__) || defined(__clang__)
#pragma message(GATEKIT_DEBUG_WARNING)
#endif

#include <iostream>
#include <vector>

namespace gatekit {
namespace detail {

template <typename T>
void dump_vector(std::string const& name, std::vector<T> const& clauses)
{
  std::cout << name << "=[";
  for (T const& item : clauses) {
    std::cout << "( ";

    for (auto lit : iterate(item)) {
      std::cout << lit << " ";
    }

    std::cout << ")";
  }
  std::cout << "] ";
}

#define GK_STRING(s) #s
#define GK_DUMP(s)                                                                                 \
  {                                                                                                \
    std::cout << GK_STRING(s) << ": " << s << "\n";                                                \
  }

}
}

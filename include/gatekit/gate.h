/**
 * \file
 *
 * Data types for gate structure representation
 */

#pragma once

#include "traits.h"

#include <cstdint>
#include <vector>

namespace gatekit {

template <typename ClauseHandle>
struct gate {
  using lit = typename clause_traits<ClauseHandle>::lit;
  using clause_handle = ClauseHandle;

  std::vector<ClauseHandle> clauses;
  std::vector<lit> inputs;
  lit output = lit{};

  uint32_t num_fwd_clauses = 0;
  bool is_nested_monotonically = false;
};

template <typename Clause>
struct gate_structure {
  using lit = typename clause_traits<Clause>::lit;

  std::vector<gate<Clause>> gates;
  std::vector<lit> roots;
};

}

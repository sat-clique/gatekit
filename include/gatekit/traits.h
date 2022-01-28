/**
 * \file
 *
 * \brief Traits for clauses and literals
 *
 * This file contains traits that need to be specialized for the concrete
 * CNF clause and literal types used by the clients of the gate analysis
 * functions.
 */

#pragma once

#include <cstdint>
#include <utility>

namespace gatekit {

template <typename ClauseHandle>
struct clause_traits {
  using lit = int;
  using size_type = std::size_t;

  static auto get(ClauseHandle clause, size_type index) -> lit { return (*clause)[index]; }

  static auto iterate(ClauseHandle clause) -> decltype(*clause) { return *clause; }

  static auto size(ClauseHandle clause) -> size_type { return clause->size(); }
};

template <typename Lit>
struct lit_traits {
  static auto negate(Lit literal) -> Lit { return -literal; }

  static auto to_index(Lit literal) -> std::size_t
  {
    if (literal >= 0) {
      return 2 * literal;
    }
    return (-2 * literal) + 1;
  }

  static auto to_var_index(Lit literal) -> std::size_t { return literal >= 0 ? literal : -literal; }
};

}

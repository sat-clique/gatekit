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

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace gatekit {

/**
 * Type traits for clauses in gatekit. The default implementation is suitable
 * for container-like clauses, with `ClauseHandle` simply being pointers to
 * clauses.
 *
 * This can also be used with more elaborate clause handle types - for example,
 * in modern SAT solvers, a clause handle might contain either the pointer to a
 * clause or it might contain up to 2 literals, directly representing a
 * unary or binary clause.
 */
template <typename ClauseHandle>
struct clause_traits {
  using lit = typename std::decay<decltype(*(std::declval<ClauseHandle>()->begin()))>::type;
  using size_type = std::size_t;

  static auto get(ClauseHandle clause, size_type index) -> lit { return (*clause)[index]; }

  static auto iterate(ClauseHandle clause) -> decltype(*clause) { return *clause; }

  static auto size(ClauseHandle clause) -> size_type { return clause->size(); }
};

/**
 * Type traits for literals in gatekit. The default implementation is suitable
 * for integer-like, DIMACS-style literals.
 */
template <typename Lit>
struct lit_traits {
  static auto negate(Lit literal) -> Lit
  {
    assert(literal != 0);
    return -literal;
  }

  /**
   * Gets a non-negative index for the given literal.
   *
   * The distance between the indices of `literal` and `-literal` must be 1.
   */
  static auto to_index(Lit literal) -> std::size_t
  {
    assert(literal != 0);

    if (literal > 0) {
      return 2 * (literal - 1);
    }
    return (-2 * (literal + 1)) + 1;
  }

  static auto to_lit(std::size_t var_index, bool positive) -> Lit
  {
    return static_cast<int>(var_index + 1) * (positive ? 1 : -1);
  }

  static auto to_var_index(Lit literal) -> std::size_t
  {
    assert(literal != 0);
    return literal > 0 ? literal - 1 : -literal - 1;
  }

  static auto is_positive(Lit literal) -> bool { return literal >= 0; }
};

}

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
 *
 * For usage with gatekit, `std::hash` must have been specialized for
 * `ClauseHandle`.
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
  static auto negate(Lit literal) -> Lit { return -literal; }

  /**
   * Gets a non-negative index for the given literal.
   *
   * The distance between the indices of `literal` and `-literal` must be 1.
   */
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

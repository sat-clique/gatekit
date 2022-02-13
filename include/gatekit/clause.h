/**
 * \file
 *
 * \brief Functions for accessing client-code clauses and literals
 *
 * This file contains structs that need to be specialized for the concrete
 * CNF clause and literal types used by the clients of gatekit. The default
 * implementations work out-of-the box for clauses that have an interface
 * similar to std::vector, and for literal types that are similar to
 * DIMACS-style integer literals.
 */

#pragma once

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace gatekit {

/**
 * Functions for accessing clauses in gatekit. The default implementation is suitable
 * for container-like clauses like `std::vector<int>`, with `ClauseHandle` simply being
 * pointers to clauses.
 *
 * For clauses that are not STL-container-like, accessor functions need to be
 * provided by specializing this struct (similar to the example given for
 * lit_funcs).
 *
 * This can also be used with more elaborate clause handle types - for example,
 * in modern SAT solvers, a clause handle might contain either the pointer to a
 * clause or it might contain up to 2 literals, directly representing a
 * unary or binary clause.
 */
template <typename ClauseHandle>
struct clause_funcs {
  using lit = typename std::decay<decltype(*(std::declval<ClauseHandle>()->begin()))>::type;
  using size_type = std::size_t;

  static auto get(ClauseHandle clause, size_type index) -> lit { return (*clause)[index]; }

  static auto iterate(ClauseHandle clause) -> decltype(*clause) { return *clause; }

  static auto size(ClauseHandle clause) -> size_type { return clause->size(); }
};

/**
 * Functions for accessing literals in gatekit. The default implementation is suitable
 * for integer-like, DIMACS-style literals.
 *
 * For other kinds of literals (for example, cnfkit or Minisat literal types),
 * this struct needs to be specialized in client code. Example for cnfkit:
 *
 * namespace gatekit {
 * template <>
 * struct lit_funcs<cnfkit::lit> {
 *   static auto negate(cnfkit::lit literal) -> cnfkit::lit { return -literal; }
 *   static auto to_index(cnfkit::lit literal) -> std::size_t { return literal.get_raw_value(); }
 *   static auto to_var_index(cnfkit::lit literal) -> std::size_t
 *   {
 *     return literal.get_var().get_raw_value();
 *   }
 * };
 * }
 */
template <typename Lit>
struct lit_funcs {
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

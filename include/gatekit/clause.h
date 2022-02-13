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
 * provided by specializing this struct.
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
 * Needs to be specialized if the client code uses literals that are not
 * DIMACS-style integer literals
 */
template <typename Lit>
auto lit_to_dimacs(Lit literal) -> int
{
  assert(literal != 0);
  return literal;
}

/**
 * Needs to be specialized if the client code uses literals that are not
 * DIMACS-style integer literals
 */
template <typename Lit>
auto dimacs_to_lit(int dimacs_literal) -> Lit
{
  assert(dimacs_literal != 0);
  return dimacs_literal;
}
}

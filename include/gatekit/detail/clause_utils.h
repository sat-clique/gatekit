#pragma once

#include <algorithm>
#include <cstddef>
#include <gatekit/clause.h>

namespace gatekit {
namespace detail {

template <typename Lit>
auto to_index(Lit lit) -> std::size_t
{
  int const dimacs_lit = lit_to_dimacs(lit);
  if (dimacs_lit > 0) {
    return 2 * (dimacs_lit - 1);
  }
  else {
    return (-2 * (dimacs_lit + 1)) + 1;
  }
}

template <typename Lit>
auto to_var_index(Lit lit) -> std::size_t
{
  int const dimacs_lit = lit_to_dimacs(lit);
  return (dimacs_lit > 0 ? dimacs_lit : -dimacs_lit) - 1;
}

template <typename Lit>
auto is_positive(Lit lit) -> bool
{
  return lit_to_dimacs(lit) > 0;
}

template <typename Lit>
auto to_lit(std::size_t var_index, bool positive) -> Lit
{
  return dimacs_to_lit<Lit>(static_cast<int>((var_index + 1) * (positive ? 1 : -1)));
}

template <typename Lit>
auto negate(Lit lit) -> Lit
{
  return dimacs_to_lit<Lit>(-lit_to_dimacs(lit));
}

template <typename Lit>
auto max_index(Lit lit) -> std::size_t
{
  return std::max(to_index(lit), to_index(negate(lit)));
}

template <typename ClauseHandle>
auto get_lit(ClauseHandle clause, typename clause_funcs<ClauseHandle>::size_type index) ->
    typename clause_funcs<ClauseHandle>::lit
{
  return clause_funcs<ClauseHandle>::get(clause, index);
}

template <typename ClauseHandle>
auto get_size(ClauseHandle clause) -> typename clause_funcs<ClauseHandle>::size_type
{
  return clause_funcs<ClauseHandle>::size(clause);
}


template <typename ClauseHandle>
auto iterate(ClauseHandle clause) -> decltype(clause_funcs<ClauseHandle>::iterate(clause))
{
  return clause_funcs<ClauseHandle>::iterate(clause);
}

}
}

#pragma once

#include <algorithm>
#include <cstddef>
#include <gatekit/clause.h>

namespace gatekit {
namespace detail {

template <typename Lit>
auto to_index(Lit lit) -> std::size_t
{
  return lit_funcs<Lit>::to_index(lit);
}

template <typename Lit>
auto to_var_index(Lit lit) -> std::size_t
{
  return lit_funcs<Lit>::to_var_index(lit);
}

template <typename Lit>
auto is_positive(Lit lit) -> bool
{
  return lit_funcs<Lit>::is_positive(lit);
}

template <typename Lit>
auto to_lit(std::size_t var_index, bool positive) -> Lit
{
  return lit_funcs<Lit>::to_lit(var_index, positive);
}

template <typename Lit>
auto negate(Lit lit) -> Lit
{
  return lit_funcs<Lit>::negate(lit);
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

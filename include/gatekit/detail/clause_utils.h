#pragma once

#include <algorithm>
#include <cstddef>
#include <gatekit/traits.h>

namespace gatekit {
namespace detail {

template <typename Lit>
auto to_index(Lit lit) -> std::size_t
{
  return lit_traits<Lit>::to_index(lit);
}

template <typename Lit>
auto to_var_index(Lit lit) -> std::size_t
{
  return lit_traits<Lit>::to_var_index(lit);
}

template <typename Lit>
auto is_positive(Lit lit) -> bool
{
  return lit_traits<Lit>::is_positive(lit);
}

template <typename Lit>
auto to_lit(std::size_t var_index, bool positive) -> Lit
{
  return lit_traits<Lit>::to_lit(var_index, positive);
}

template <typename Lit>
auto negate(Lit lit) -> Lit
{
  return lit_traits<Lit>::negate(lit);
}

template <typename Lit>
auto max_index(Lit lit) -> std::size_t
{
  return std::max(to_index(lit), to_index(negate(lit)));
}

template <typename ClauseHandle>
auto get_lit(ClauseHandle clause, typename clause_traits<ClauseHandle>::size_type index) ->
    typename clause_traits<ClauseHandle>::lit
{
  return clause_traits<ClauseHandle>::get(clause, index);
}

template <typename ClauseHandle>
auto get_size(ClauseHandle clause) -> typename clause_traits<ClauseHandle>::size_type
{
  return clause_traits<ClauseHandle>::size(clause);
}


template <typename ClauseHandle>
auto iterate(ClauseHandle clause) -> decltype(clause_traits<ClauseHandle>::iterate(clause))
{
  return clause_traits<ClauseHandle>::iterate(clause);
}

}
}

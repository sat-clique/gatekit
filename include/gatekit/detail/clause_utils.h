#pragma once

#include <gatekit/traits.h>

namespace gatekit {
namespace detail {

template <typename Lit>
auto to_index(Lit lit)
{
  return lit_traits<Lit>::to_index(lit);
}

template <typename Lit>
auto to_var_index(Lit lit)
{
  return lit_traits<Lit>::to_var_index(lit);
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

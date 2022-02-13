#pragma once

#include <gatekit/clause.h>
#include <gatekit/detail/clause_utils.h>
#include <gatekit/detail/occurrence_list.h>

namespace gatekit {
namespace detail {

template <typename ClauseHandle>
auto is_resolvent_tautologic(ClauseHandle lhs,
                             ClauseHandle rhs,
                             typename clause_funcs<ClauseHandle>::lit const& resolution_lit) -> bool
{
  std::size_t const resolution_idx = to_var_index(resolution_lit);

  for (auto lhs_lit : iterate(lhs)) {
    if (to_var_index(lhs_lit) == resolution_idx) {
      continue;
    }

    for (auto rhs_lit : iterate(rhs)) {
      if (lhs_lit == negate(rhs_lit)) {
        return true;
      }
    }
  }

  return false;
}


template <typename OccList>
auto is_blocked(typename OccList::lit lit, OccList const& clauses) -> bool
{
  for (auto const& fwd_clause : clauses[negate(lit)]) {
    for (auto const& bwd_clause : clauses[lit]) {
      if (!is_resolvent_tautologic(fwd_clause, bwd_clause, lit)) {
        return false;
      }
    }
  }

  return true;
}

}
}

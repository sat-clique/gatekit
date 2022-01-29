#pragma once

#include <gatekit/detail/blocked_set.h>
#include <gatekit/detail/clause_utils.h>
#include <gatekit/detail/occurrence_list.h>

#include <gatekit/gate.h>
#include <gatekit/traits.h>

#include <algorithm>
#include <unordered_set>

namespace gatekit {
namespace detail {

template <typename OccList>
auto is_output_of_fully_encoded_gate(typename OccList::lit const& output, OccList const& clauses)
    -> bool
{
  // Assumption: `output` is blocked

  using lit = typename OccList::lit;

  auto const& fwd_clauses = clauses[negate(output)];
  auto const& bwd_clauses = clauses[output];

  std::unordered_set<std::size_t> fwd_vars;
  std::unordered_set<std::size_t> bwd_vars;

  std::size_t const output_var_index = to_var_index(output);

  for (auto const& fwd_clause : fwd_clauses) {
    for (lit const& fwd_lit : iterate(fwd_clause)) {
      std::size_t const fwd_var_index = to_var_index(fwd_lit);

      if (fwd_var_index != output_var_index) {
        fwd_vars.insert(to_var_index(fwd_lit));
      }
    }
  }

  for (auto const& bwd_clause : bwd_clauses) {
    for (lit const& bwd_lit : iterate(bwd_clause)) {
      std::size_t const bwd_var_index = to_var_index(bwd_lit);

      if (bwd_var_index != output_var_index) {
        if (std::find(fwd_vars.begin(), fwd_vars.end(), bwd_var_index) == fwd_vars.end()) {
          return false;
        }
        bwd_vars.insert(bwd_var_index);
      }
    }
  }

  return fwd_vars.size() == bwd_vars.size();
}

template <typename OccList>
auto is_gate_output(typename OccList::lit const& output,
                    OccList const& clauses,
                    bool is_nested_monotonically) -> bool
{
  if (clauses[negate(output)].empty()) {
    // `output` is not a gate output, since the possible inputs cannot
    // constrain it.
    return false;
  }

  if (!is_blocked(output, clauses)) {
    // The clauses currently remaining in the occurrence list
    // are not a gate encoding, since CNF gate encodings are
    // required to be a blocked set (with `output` being a
    // blocked literal).
    //
    // Note that as long as the output variable is used as
    // input for any other gate G whose clauses are still in the
    // occurrence list, this check fails, even if `output` is
    // indeed the output of a gate. G needs to be recovered first,
    // so that its clauses are not contained in the occurrence
    // list anymore.
    return false;
  }

  if (is_nested_monotonically) {
    // For gates that are nested monotonically, only the clauses
    // containing `-output` are required to be encoded in CNF.
    // Other clauses are irrelevant for the functional relationship
    // between input and output. Since we already checked that there
    // are clauses containing `-output`, the clauses associated
    // with `output` indeed form a gate:
    return true;
  }

  return is_output_of_fully_encoded_gate(output, clauses);
}

}
}

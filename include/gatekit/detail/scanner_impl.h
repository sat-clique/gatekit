#pragma once

#include <gatekit/detail/blocked_set.h>
#include <gatekit/detail/clause_utils.h>
#include <gatekit/detail/collections.h>
#include <gatekit/detail/occurrence_list.h>
#include <gatekit/detail/scanner_single.h>

#include <gatekit/gate.h>
#include <gatekit/traits.h>

#include <algorithm>
#include <unordered_set>
#include <vector>


namespace gatekit {
namespace detail {

template <typename ClauseHandle>
struct optional_gate {
  gate<ClauseHandle> m_gate;
  bool m_is_valid = false;
};

template <typename ClauseHandle>
auto get_inputs(gate<ClauseHandle> const& gate)
    -> std::vector<typename clause_traits<ClauseHandle>::lit>
{
  using lit = typename clause_traits<ClauseHandle>::lit;

  std::vector<lit> result;

  auto const stop = gate.clauses.begin() + gate.num_fwd_clauses;
  for (auto iter = gate.clauses.begin(); iter != stop; ++iter) {
    for (lit const& literal : iterate(*iter)) {
      if (to_var_index(literal) == to_var_index(gate.output)) {
        continue;
      }

      if (std::find(result.begin(), result.end(), literal) == result.end()) {
        result.push_back(literal);
      }
    }
  }

  return result;
}

template <typename OccList>
auto create_valid_gate(typename OccList::lit const& output,
                       OccList const& clauses,
                       bool is_nested_monotonically)
    -> optional_gate<typename OccList::clause_handle>
{
  using ClauseHandle = typename OccList::clause_handle;

  optional_gate<ClauseHandle> result;

  result.m_is_valid = true;
  result.m_gate.output = output;
  result.m_gate.is_nested_monotonically = is_nested_monotonically;
  result.m_gate.clauses = clauses[negate(output)];
  result.m_gate.num_fwd_clauses = result.m_gate.clauses.size();

  auto const& bwd_clauses = clauses[output];
  result.m_gate.clauses.insert(result.m_gate.clauses.end(), bwd_clauses.begin(), bwd_clauses.end());

  result.m_gate.inputs = get_inputs(result.m_gate);

  return result;
}


template <typename OccList>
auto try_get_gate(typename OccList::lit const& output,
                  OccList const& clauses,
                  bool is_nested_monotonically) -> optional_gate<typename OccList::clause_handle>
{
  if (is_gate_output(output, clauses, is_nested_monotonically)) {
    return create_valid_gate(output, clauses, is_nested_monotonically);
  }

  return {};
}

template <typename Lit, typename OccList>
void sort_by_estimated_access_cost(std::vector<Lit>& to_sort, OccList const& occs)
{
  std::sort(to_sort.begin(), to_sort.end(), [&occs](Lit const& lhs, Lit const& rhs) {
    return occs.get_estimated_lookup_cost(lhs) < occs.get_estimated_lookup_cost(rhs);
  });
}

template <typename ClauseHandle>
void extend_gate_structure(gate_structure<ClauseHandle>& result,
                           occurrence_list<ClauseHandle>& occs,
                           typename clause_traits<ClauseHandle>::lit root)
{
  using lit = typename clause_traits<ClauseHandle>::lit;

  // Basic algorithm:
  //
  // The gate structure is scanned using BFS. The inputs of gates
  // discovered in the current round are used as current_candidates
  // in the next round. Initially, only the root literal is a candidate.
  // When a gate is found, its clauses are removed from the occurrence
  // lists, and the search continues.
  //
  // If no gate is found for a given element X of current_candidates,
  // X is either really not the output of a gate (then, we can ignore
  // it), or it is the output of a gate that cannot be recognized by
  // the current implementation (~> also ignored), or it could be
  // recognized but is used as input of some gate that needs to be
  // recovered first, causing the occurrence lists of ~X and X to
  // contain clauses of other gates. In that case, X automatically
  // becomes a candidate again when the last gate having X or ~X as
  // input has been recovered.

  std::vector<lit> current_candidates = {root};
  literal_set<lit> next_candidates{occs.get_max_lit_index()};
  literal_set<lit> inputs{occs.get_max_lit_index()};

  bool found_any = false;

  while (!current_candidates.empty()) {
    // Preferring to access "cheap" literals first, causing removals to be performed in
    // the occurrence list while it is cheap. This causes the cost of cheap
    // occurrence_list<>::operator[]() to become even cheaper (most cases) and further
    // increases the cost of expensive occurrence_list<>::operator[]() calls (rare
    // enough).
    //
    // For 9dlx_vliw_at_b_iq8 (GBD hash be6bce96bab9cec47b58b6c68c9a1c89),
    // this optimization causes a speedup of >30%, for 13pipe_k (GBD hash
    // 772102b16ea3acaf7b516714b146b6ca) the speedup is ~10%.
    sort_by_estimated_access_cost(current_candidates, occs);

    for (lit candidate : current_candidates) {
      bool const is_nonmono = inputs.contains(candidate) && inputs.contains(negate(candidate));

      optional_gate<ClauseHandle> potential_gate = try_get_gate(candidate, occs, !is_nonmono);

      if (potential_gate.m_is_valid) {
        occs.remove_gate_root(potential_gate.m_gate.output);

        next_candidates.add_all(potential_gate.m_gate.inputs);
        inputs.add_all(potential_gate.m_gate.inputs);

        if (!potential_gate.m_gate.is_nested_monotonically) {
          for (lit input_lit : potential_gate.m_gate.inputs) {
            inputs.add(negate(input_lit));
          }
        }

        result.gates.push_back(std::move(potential_gate.m_gate));
        found_any = true;
      }
    }

    current_candidates = next_candidates.literals();
    next_candidates.clear();
  }

  if (found_any) {
    result.roots.push_back({root});
  }
}


template <typename ClauseHandle, typename ClauseHandleIter>
auto scan_gates_impl(ClauseHandleIter start, ClauseHandleIter stop) -> gate_structure<ClauseHandle>
{
  occurrence_list<ClauseHandle> occs{start, stop};

  gate_structure<ClauseHandle> result;

  auto unaries = occs.get_unaries();
  for (auto root_candidate : unaries) {
    occs.remove_unary(root_candidate);
    extend_gate_structure(result, occs, root_candidate);
  }

  return result;
}

}
}

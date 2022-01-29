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

template <typename ClauseHandle>
void extend_gate_structure(gate_structure<ClauseHandle>& result,
                           occurrence_list<ClauseHandle>& occs,
                           typename clause_traits<ClauseHandle>::lit root)
{
  using lit = typename clause_traits<ClauseHandle>::lit;

  std::vector<lit> current_candidates = {root};
  literal_set<lit> next_candidates{occs.get_max_lit_index()};
  literal_set<lit> inputs{occs.get_max_lit_index()};

  bool found_any = false;

  while (!current_candidates.empty()) {
    for (lit candidate : current_candidates) {
      bool const is_nonmono = inputs.contains(candidate) && inputs.contains(negate(candidate));

      optional_gate<ClauseHandle> potential_gate =
          try_get_gate(negate(candidate), occs, !is_nonmono);

      if (potential_gate.m_is_valid) {
        occs.remove_all(potential_gate.m_gate.clauses);

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

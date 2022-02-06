#pragma once

#include <gatekit/detail/clause_utils.h>
#include <gatekit/detail/scanner_structure.h>
#include <gatekit/gate.h>

#include <algorithm>
#include <vector>

namespace gatekit {

template <typename ClauseHandle>
auto to_structure(std::vector<gate<ClauseHandle>>&& gates, std::vector<std::vector<int>>&& roots)
    -> gate_structure<ClauseHandle>
{
  gate_structure<ClauseHandle> result;
  result.gates = std::move(gates);
  result.roots = std::move(roots);
  return result;
}

template <typename ClauseHandle>
auto with_flipped_output_sign(gate<ClauseHandle> const& to_flip) -> gate<ClauseHandle>
{
  assert(!to_flip.is_nested_monotonically);

  gate<ClauseHandle> result;
  result.output = detail::negate(to_flip.output);

  std::copy(to_flip.clauses.begin() + to_flip.num_fwd_clauses,
            to_flip.clauses.end(),
            std::back_inserter(result.clauses));
  result.num_fwd_clauses = result.clauses.size();
  std::copy(to_flip.clauses.begin(),
            to_flip.clauses.begin() + to_flip.num_fwd_clauses,
            std::back_inserter(result.clauses));

  result.inputs = detail::get_inputs(result);

  result.is_nested_monotonically = false;

  return result;
}
}

#include "gate_factory.h"

#include <gatekit/detail/scanner_structure.h>
#include <gatekit/gate.h>

#include <algorithm>

namespace gatekit {
namespace {

auto create_gate(std::vector<Clause> const& fwd_clauses,
                 std::vector<Clause> const& bwd_clauses,
                 int output) -> gate<ClauseHandle>
{
  gate<ClauseHandle> result;

  result.num_fwd_clauses = fwd_clauses.size();
  result.output = output;

  for (auto const& clause : fwd_clauses) {
    result.clauses.emplace_back(std::make_shared<Clause>(clause));
  }

  for (auto const& clause : bwd_clauses) {
    result.clauses.emplace_back(std::make_shared<Clause>(clause));
  }

  result.inputs = ::gatekit::detail::get_inputs(result);

  return result;
}
}

auto and_gate(std::vector<int> const& inputs, int output) -> gate<ClauseHandle>
{
  std::vector<int> bwd_clause = {output};
  std::vector<std::vector<int>> fwd_clauses;

  for (int input : inputs) {
    bwd_clause.push_back(-input);
    fwd_clauses.push_back({input, -output});
  }

  return create_gate(fwd_clauses, {bwd_clause}, output);
}

auto or_gate(std::vector<int> const& inputs, int output) -> gate<ClauseHandle>
{
  std::vector<int> fwd_clause = {-output};
  std::vector<std::vector<int>> bwd_clauses;

  for (int input : inputs) {
    fwd_clause.push_back(input);
    bwd_clauses.push_back({-input, output});
  }

  return create_gate({fwd_clause}, bwd_clauses, output);
}

auto xor_gate(int lhs, int rhs, int output) -> gate<ClauseHandle>
{
  return create_gate({{-output, -lhs, -rhs}, {-output, lhs, rhs}},
                     {{output, -lhs, rhs}, {output, lhs, -rhs}},
                     output);
}

auto monotonic(gate<ClauseHandle>&& gate, encoding encoding) -> ::gatekit::gate<ClauseHandle>
{
  if (encoding == encoding::opt) {
    // keep only fwd clauses
    gate.clauses.resize(gate.num_fwd_clauses);
  }

  gate.is_nested_monotonically = true;
  return std::move(gate);
}
}

#include "gate_factory.h"

#include <gatekit/gate.h>

#include <algorithm>

namespace gatekit {
namespace {
auto create_gate(std::vector<int> const& inputs,
                 std::vector<Clause> const& fwd_clauses,
                 std::vector<Clause> const& bwd_clauses,
                 int output) -> gate<ClauseHandle>
{
  gate<ClauseHandle> result;

  result.inputs = inputs;
  result.num_fwd_clauses = fwd_clauses.size();
  result.output = output;

  for (auto const& clause : fwd_clauses) {
    result.clauses.emplace_back(std::make_shared<Clause>(clause));
  }

  for (auto const& clause : bwd_clauses) {
    result.clauses.emplace_back(std::make_shared<Clause>(clause));
  }

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

  return create_gate(inputs, fwd_clauses, {bwd_clause}, output);
}

auto or_gate(std::vector<int> const& inputs, int output) -> gate<ClauseHandle>
{
  std::vector<int> neg_inputs = inputs;
  for (int& input : neg_inputs) {
    input *= -1;
  }

  return and_gate(neg_inputs, -output);
}

auto xor_gate(int lhs, int rhs, int output) -> gate<ClauseHandle>
{
  return create_gate({lhs, rhs},
                     {{-output, -lhs, -rhs}, {-output, lhs, rhs}},
                     {{output, -lhs, rhs}, {output, lhs, -rhs}},
                     output);
}

auto monotonic(gate<ClauseHandle>&& gate, polarity polarity) -> ::gatekit::gate<ClauseHandle>
{
  if (polarity == polarity::pos) {
    // keep only fwd clauses
    gate.clauses.resize(gate.num_fwd_clauses);
  }
  else {
    // keep only bwd clauses
    gate.clauses.erase(gate.clauses.begin(), gate.clauses.begin() + gate.num_fwd_clauses);
  }

  gate.is_nested_monotonically = true;
  return std::move(gate);
}
}

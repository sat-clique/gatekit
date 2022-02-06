#pragma once

#include <gatekit/detail/bitvector.h>
#include <gatekit/gate.h>

#include <utility>

namespace gatekit {
namespace detail {

template <typename ClauseHandle>
class prop_clauses {
public:
  using iterator = decltype(std::declval<gate<ClauseHandle>>().clauses.cbegin());

  prop_clauses(gate<ClauseHandle> const& gate)
  {
    bool const has_fewer_fwd = (gate.clauses.size() - gate.num_fwd_clauses <= gate.num_fwd_clauses);
    m_iterating_fwd = gate.is_nested_monotonically || has_fewer_fwd;

    if (m_iterating_fwd) {
      m_begin = gate.clauses.begin();
      m_end = gate.clauses.begin() + gate.num_fwd_clauses;
    }
    else {
      m_begin = gate.clauses.begin() + gate.num_fwd_clauses;
      m_end = gate.clauses.end();
    }
  }

  auto begin() const noexcept -> iterator { return m_begin; }

  auto end() const noexcept -> iterator { return m_end; }

  auto is_iterating_fwd() const noexcept -> bool { return m_iterating_fwd; }

private:
  bool m_iterating_fwd = false;
  iterator m_begin;
  iterator m_end;
};


template <typename ClauseHandle, std::size_t Bits, std::size_t Alignment>
void propagate_gate(bitvector_map<Bits, Alignment>& assignment_by_var,
                    gate<ClauseHandle> const& gate)
{
  using bitvec = bitvector<Bits, Alignment>;

  auto const out_var = to_var_index(gate.output);

  // Approach: check if fwd (rsp. the bwd clauses, whichever set is smaller) are all
  // satisfied under the current assignment (without regarding the output
  // variable) and store the result in the bitvector output_forced_by_other_set.
  // This determines whether the fwd or the bwd clauses force an assignment on
  // the gate output.
  //
  // For example, if the N'th bit of output_forced_by_other_set is true and
  // the bwd clauses have been checked, some fwd clause must force the literal
  // `-(gate.output)` to be true under the N'th input variable assignment. Conversely,
  // if the N'th bit of output_forced_by_other_set is false, some bwd clause
  // forces the literal `gate.output` to be true under the N'th input variable
  // assignment.
  //
  // For monotonically nested gates, the fwd clauses are always checked, since
  // those gates are not required to have a full set of bwd clauses. However,
  // the algorithm still needs to assign the literal `gate.output` true in case no
  // fwd clause forces the output (as if Plaisted-Greenbaum optimization had
  // not been applied to the gate structure) since this propagator uses a
  // binary variable assignment, so indeterminacy cannot be expressed. However,
  // this is not a problem since the original gate semantics are not violated.

  bitvec output_forced_by_other_set = bitvec::ones();

  prop_clauses<ClauseHandle> clauses{gate};
  for (ClauseHandle const& clause : clauses) {
    bitvec this_clause_satisfied = bitvec::zeros();

    for (auto const& lit : iterate(clause)) {
      auto const lit_var = to_var_index(lit);
      if (lit_var == out_var) {
        continue;
      }

      bitvec& lit_var_assignments = assignment_by_var[lit_var];

      if (is_positive(lit)) {
        this_clause_satisfied |= lit_var_assignments;
      }
      else {
        this_clause_satisfied |= ~lit_var_assignments;
      }
    }

    output_forced_by_other_set &= this_clause_satisfied;
  }

  // fwd clauses contain -gate.output, bwd clauses contain gate.output,
  // so to get the value of the output *variable*, output_forced_by_other_set
  // needs to be inverted accordingly: if the fwd clauses force the output,
  // the n'th bit of output_forced_by_other_set is 1 iff the n'th output
  // *literal* is negative.
  bool const fwd_forces_output = !clauses.is_iterating_fwd();
  if ((fwd_forces_output && !is_positive(gate.output)) ||
      (!fwd_forces_output && is_positive(gate.output))) {
    assignment_by_var[out_var] = output_forced_by_other_set;
  }
  else {
    assignment_by_var[out_var] = ~output_forced_by_other_set;
  }
}


template <typename ClauseHandle, std::size_t Bits, std::size_t Alignment>
void propagate_structure(bitvector_map<Bits, Alignment>& assignment_by_var,
                         gate_structure<ClauseHandle> const& structure)
{
  std::vector<gate<ClauseHandle>> const& gates = structure.gates;

  // Assuming a (reverse) topological oder on `gates`: for all `0 <= n <
  // gates.size()`, the output variable `o` of gate `gates[n]` does not
  // occur in the inputs of `gates[m]` for any `m > n`.
  //
  // Incidentally, this is the order on `gates` imposed by the scanner
  // algorithm.
  //
  // Therefore, the assignment can be propagated by iterating backwards
  // through `gates`, with each gate forcing its output:
  for (auto gate_iter = gates.rbegin(); gate_iter != gates.rend(); ++gate_iter) {
    propagate_gate(assignment_by_var, *gate_iter);
  }
}

}
}

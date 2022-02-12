#pragma once

#include <gatekit/detail/blocked_set.h>
#include <gatekit/detail/clause_utils.h>
#include <gatekit/detail/occurrence_list.h>
#include <gatekit/detail/utils.h>

#include <gatekit/gate.h>
#include <gatekit/traits.h>

#include <algorithm>
#include <vector>

namespace gatekit {
namespace detail {

template <typename OccList>
auto try_get_gate_inputs(typename OccList::lit const& output, OccList const& clauses)
    -> std::vector<std::size_t>
{
  // Assumption: `output` is blocked
  //
  // Note: this check rarely returns false (ca. 0.1% to 1.5% of all calls) since blockedness
  // has already been checked. Unfortunately, the code may only err on the side
  // of returning false.

  using lit = typename OccList::lit;

  auto const& fwd_clauses = clauses[negate(output)];
  auto const& bwd_clauses = clauses[output];

  std::vector<std::size_t> fwd_vars;
  std::vector<std::size_t> bwd_vars;

  std::size_t const output_var_index = to_var_index(output);

  for (auto const& fwd_clause : fwd_clauses) {
    for (lit const& fwd_lit : iterate(fwd_clause)) {
      std::size_t const fwd_var_index = to_var_index(fwd_lit);

      if (fwd_var_index != output_var_index) {
        if (std::find(fwd_vars.begin(), fwd_vars.end(), fwd_var_index) == fwd_vars.end()) {
          fwd_vars.push_back(fwd_var_index);
        }
      }
    }
  }

  for (auto const& bwd_clause : bwd_clauses) {
    for (lit const& bwd_lit : iterate(bwd_clause)) {
      std::size_t const bwd_var_index = to_var_index(bwd_lit);

      if (bwd_var_index != output_var_index) {
        if (std::find(bwd_vars.begin(), bwd_vars.end(), bwd_var_index) == bwd_vars.end()) {
          bwd_vars.push_back(bwd_var_index);
        }
      }
    }
  }

  if (fwd_vars.size() != bwd_vars.size()) {
    return {};
  }

  std::sort(fwd_vars.begin(), fwd_vars.end());
  std::sort(bwd_vars.begin(), bwd_vars.end());

  if (fwd_vars == bwd_vars) {
    return fwd_vars;
  }
  else {
    return {};
  }
}

template <typename ClauseHandle>
auto are_all_of_size(std::vector<ClauseHandle> const& clauses, std::size_t size) -> bool
{
  return std::all_of(clauses.begin(), clauses.end(), [size](ClauseHandle const& clause) {
    return get_size(clause) == size;
  });
}

template <typename ClauseHandle>
auto get_num_covered_input_combinations(std::vector<ClauseHandle> const& clauses,
                                        std::size_t num_inputs) -> uint64_t
{
  uint64_t result = 0;

  for (ClauseHandle clause : clauses) {
    result += 1ull << (num_inputs + 1 - get_size(clause));
  }

  return result;
}

template <typename ClauseHandle>
auto is_joined_clause_taut(ClauseHandle const& lhs, ClauseHandle const& rhs) -> bool
{
  for (auto const& lit_lhs : iterate(lhs)) {
    for (auto const& lit_rhs : iterate(rhs)) {
      if (lit_lhs == negate(lit_rhs)) {
        return true;
      }
    }
  }

  return false;
}


template <typename ClauseHandle>
auto are_pairwise_joined_clauses_all_taut(std::vector<ClauseHandle> const& clauses,
                                          std::size_t num_inputs) -> bool
{
  for (ClauseHandle const& lhs : clauses) {
    if (get_size(lhs) == num_inputs + 1) {
      continue;
    }

    for (ClauseHandle const& rhs : clauses) {
      if (&lhs == &rhs || get_size(rhs) == num_inputs + 1) {
        continue;
      }

      if (!is_joined_clause_taut(lhs, rhs)) {
        return false;
      }
    }
  }

  return true;
}


template <typename OccList>
auto is_full_gate_or_ssr_optimized(typename OccList::lit const& output,
                                   OccList const& clauses,
                                   std::vector<size_t> const& inputs) -> bool
{
  // Detect gates in which each input assignment causes exactly one clause
  // to propagate the output. XOR gates and gates with one clause for each
  // possible input assignment are special cases of this class of encoded
  // gates.
  //
  // Additionally, this matcher recognizes gate encodings in which literals
  // are omitted from clauses via self-subsuming resolution (i.e. because
  // they are don't-cares relative to some partial assignment of inputs),
  // like
  //    (a, b, -o)
  //      (-b, -o)
  //   (-a, b,  o)
  // where the assignment of `a` is irrelevant for `o` when `b` is `true`.
  //
  // The basic idea of the check is to compute the number of different input
  // assignment "covered" by each clause and comparing it to the total
  // number of different input assignments for the gate. Also, if A u B is
  // tautologic for each two distinct clauses A, B in the gate encoding,
  // each input causes a single gate in the clause to propagate the output.

  using ClauseHandle = typename OccList::clause_handle;

  std::vector<ClauseHandle> const& fwd = clauses[negate(output)];
  std::vector<ClauseHandle> const& bwd = clauses[output];
  std::size_t const num_inputs = inputs.size();

  if (num_inputs <= 63) {
    uint64_t const num_total_input_combinations = (1ull << num_inputs);
    uint64_t const num_covered_input_combinations =
        get_num_covered_input_combinations(fwd, num_inputs) +
        get_num_covered_input_combinations(bwd, num_inputs);

    if (num_covered_input_combinations == num_total_input_combinations) {
      // If a clause contains A contains x and B contains -x, they
      // cannot propagate their output literal at the same time. fwd
      // and bwd can be checked separately because blockedness already
      // guarantees right-uniqueness.
      if (are_pairwise_joined_clauses_all_taut(fwd, num_inputs) &&
          are_pairwise_joined_clauses_all_taut(bwd, num_inputs)) {
        return true;
      }
    }
  }

  return false;
}


template <typename ClauseHandle>
auto get_clause_sizes_if_same_length(std::vector<ClauseHandle> const& clauses) -> std::size_t
{
  if (clauses.empty()) {
    return 0;
  }

  std::size_t result = get_size(clauses.front());

  for (auto const& clause : clauses) {
    if (get_size(clause) != result) {
      return 0;
    }
  }

  return result;
}


template <typename OccList>
auto is_at_least_k_gate(typename OccList::lit const& output,
                        OccList const& clauses,
                        std::vector<size_t> const& inputs) -> bool
{
  using ClauseHandle = typename OccList::clause_handle;

  std::vector<ClauseHandle> const& fwd = clauses[negate(output)];
  std::vector<ClauseHandle> const& bwd = clauses[output];

  std::size_t const fwd_clause_size = get_clause_sizes_if_same_length(fwd);
  if (fwd_clause_size == 0) {
    return false;
  }

  std::size_t const bwd_clause_size = get_clause_sizes_if_same_length(bwd);
  if (bwd_clause_size == 0) {
    return false;
  }

  std::size_t const k = bwd_clause_size - 1;
  std::size_t const anti_k = inputs.size() - k + 1;

  if (fwd_clause_size != anti_k + 1) {
    return false;
  }

  std::size_t const inputs_choose_k = n_choose_k(inputs.size(), k);
  std::size_t const inputs_choose_anti_k = n_choose_k(inputs.size(), anti_k);

  return inputs_choose_k == bwd.size() && inputs_choose_anti_k == fwd.size();
}


template <typename OccList>
auto is_matching_gate_pattern(typename OccList::lit const& output,
                              OccList const& clauses,
                              std::vector<size_t> const& inputs) -> bool
{
  // Note that
  //   * AND and OR gates are special cases of at-least-k gates
  //   * at-most-k gates can be interpreted in terms of at-least-k'
  //   * XOR gates are special cases of "full" gates
  return is_at_least_k_gate(output, clauses, inputs) ||
         is_full_gate_or_ssr_optimized(output, clauses, inputs);
}

template <typename OccList>
auto is_output_of_fully_encoded_gate(typename OccList::lit const& output, OccList const& clauses)
    -> bool
{
  std::vector<size_t> inputs = try_get_gate_inputs(output, clauses);
  return !inputs.empty() && is_matching_gate_pattern(output, clauses, inputs);
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

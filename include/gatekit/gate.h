/**
 * \file
 *
 * Data types for gate structure representation
 */

#pragma once

#include <gatekit/clause.h>
#include <gatekit/detail/clause_utils.h>
#include <gatekit/detail/utils.h>

#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

namespace gatekit {

/**
 * \brief Data structure for CNF gate encodings
 *
 * A gate represents a functional relationship between input and output variables in
 * propositional logic formulas rsp. SAT problem instances.
 *
 * For an in-depth formal definition of gates and gate structures, see
 *
 * Iser M., Manthey N., Sinz C. (2015): Recognition of Nested Gates in CNF Formulas.
 * In: Heule M., Weaver S. (eds) Theory and Applications of Satisfiability Testing -- SAT
 * 2015. SAT 2015. Lecture Notes in Computer Science, vol 9340. Springer, Cham.
 * https://doi.org/10.1007/978-3-319-24318-4_19
 */
template <typename ClauseHandle>
struct gate {
  using lit = typename clause_funcs<ClauseHandle>::lit;
  using clause_handle = ClauseHandle;

  /**
   * CNF representation of the gate.
   *
   * The gate can be interpreted as a constraint `output <-> F` for
   * some formula `F`. The clauses encoding `output -> F` are called
   * forward clauses, all other clauses are called backward clauses.
   *
   * If the gate is nested monotonically, the backward clauses may be
   * omitted from `clauses`. `clauses` is partitioned: the forward
   * clauses precede the backward clauses.
   *
   * For more information, see also the following paper:
   *
   * David A. Plaisted, Steven Greenbaum: A Structure-preserving Clause Form Translation.
   * Journal of Symbolic Computation, Volume 2, Issue 3, 1986, Pages 293-304,
   * ISSN 0747-7171, https://doi.org/10.1016/S0747-7171(86)80028-1.
   *
   */
  std::vector<ClauseHandle> clauses;

  /**
   * The gate input literals. Each input variable occurs at least once.
   *
   * For nonmonotonically nested gates, each literal l in this list
   * is an input, as well as ~l.
   */
  std::vector<lit> inputs;

  /**
   * The gate output literal. All forward clauses contain -output.
   */
  lit output = lit{};

  /**
   * The number of forward clauses contained in `clauses`.
   */
  uint32_t num_fwd_clauses = 0;

  /**
   * `true` iff the gate is contained in a gate structure where
   * it is nested monotonically.
   */
  bool is_nested_monotonically = false;
};

/**
 * \brief A collection of gates.
 *
 * This structure can be interpreted as a DAG, with
 * gates (and the special root constraints) being nodes and an
 * edge (X -> Y) exists if and only if the output of X (or its
 * negation) is input to Y. The root constraints have no outgoing
 * edges.
 */
template <typename Clause>
struct gate_structure {
  using lit = typename clause_funcs<Clause>::lit;

  std::vector<gate<Clause>> gates;

  // Root constraints, e.g. a unary clause. May be empty.
  std::vector<std::vector<lit>> roots;
};


/**
 * \brief Returns a JSON object string representation of the given gate
 */
template <typename ClauseHandle>
auto to_string(gate<ClauseHandle> const& gate) -> std::string
{
  using std::to_string;

  std::string result = "{";
  result += "\"inputs\": " + detail::iterable_to_string(gate.inputs) + ", ";
  result += "\"output\": " + to_string(gate.output) + ", ";
  result += "\"num_fwd_clauses\": " + to_string(gate.num_fwd_clauses) + ", ";
  result += "\"is_nested_monotonically\": " + to_string(gate.is_nested_monotonically) + ", ";
  result +=
      "\"clauses\": " + detail::iterable_to_string(gate.clauses, [](ClauseHandle const& clause) {
        return detail::iterable_to_string(::gatekit::detail::iterate(clause));
      });

  result += "}";
  return result;
}

/**
 * \brief Returns a JSON object string representation of the given gate
 *        structure
 */
template <typename ClauseHandle>
auto to_string(gate_structure<ClauseHandle> const& structure) -> std::string
{
  using lit = typename clause_funcs<ClauseHandle>::lit;

  std::string result = "{";
  result += "\"gates\": " + detail::iterable_to_string(structure.gates) + ", ";
  result += "\"roots\": " +
            detail::iterable_to_string(structure.roots, [](std::vector<lit> const& roots) {
              return detail::iterable_to_string(roots);
            });

  result += "}";
  return result;
}

/**
 * \brief Returns the maximum variable index occurring in the given gate, or 0
 *        if the gate is empty
 */
template <typename ClauseHandle>
auto max_var_index(gate<ClauseHandle> const& gate) -> std::size_t
{
  std::size_t result = detail::to_var_index(gate.output);
  for (auto const& clause : gate.clauses) {
    for (auto const& lit : *clause) {
      result = std::max(result, detail::to_var_index(lit));
    }
  }
  return result;
}

/**
 * \brief Returns the maximum variable index occurring in the given gate
 *        structure, or 0 if the structure is empty.
 */
template <typename ClauseHandle>
auto max_var_index(gate_structure<ClauseHandle> const& structure) -> std::size_t
{
  std::size_t result = 0;
  for (auto const& gate : structure.gates) {
    result = std::max(result, max_var_index(gate));
  }
  return result;
}

/**
 * \brief Returns the indices of all variables that occur in any gate inputs,
 *        but are not gate outputs. The result is sorted in ascending order.
 */
template <typename ClauseHandle>
auto input_var_indices(gate_structure<ClauseHandle> const& structure) -> std::vector<std::size_t>
{
  std::unordered_set<std::size_t> seen_outputs;
  std::unordered_set<std::size_t> seen_vars;

  for (auto const& gate : structure.gates) {
    seen_outputs.insert(detail::to_var_index(gate.output));
    for (auto const& input_lit : gate.inputs) {
      seen_vars.insert(detail::to_var_index(input_lit));
    }
  }

  for (auto const& lit : seen_outputs) {
    seen_vars.erase(lit);
  }

  std::vector<std::size_t> result{seen_vars.begin(), seen_vars.end()};
  std::sort(result.begin(), result.end());

  return result;
}

}

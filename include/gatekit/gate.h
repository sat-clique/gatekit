/**
 * \file
 *
 * Data types for gate structure representation
 */

#pragma once

#include "traits.h"

#include <cstdint>
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
  using lit = typename clause_traits<ClauseHandle>::lit;
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
  using lit = typename clause_traits<Clause>::lit;

  std::vector<gate<Clause>> gates;

  // Root constraints, e.g. a unary clause. May be empty.
  std::vector<std::vector<lit>> roots;
};

}

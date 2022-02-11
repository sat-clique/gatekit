#pragma once

#include <gatekit/gate.h>

#include <gatekit/detail/bitvector.h>
#include <gatekit/detail/bitvector_partition.h>
#include <gatekit/detail/bitvector_prop.h>
#include <gatekit/detail/bitvector_rand.h>

namespace gatekit {

namespace detail {
template <std::size_t Bits, std::size_t Alignment>
void randomize(bitvector_map<Bits, Alignment>& assignments,
               bitvector_randomizer& randomizer,
               std::vector<std::size_t> const& input_var_indices,
               uint64_t step)
{
  if (step % 2 == 0) {
    for (std::size_t var : input_var_indices) {
      randomizer.randomize(assignments[var], ((step / 2) % 7) + 1);
    }
  }
  else {
    for (std::size_t var : input_var_indices) {
      assignments[var] = ~assignments[var];
    }
  }
}

template <std::size_t Bits, std::size_t Alignment>
void randomize_all(bitvector_map<Bits, Alignment>& assignments, bitvector_randomizer& randomizer)
{
  for (std::size_t idx = 0; idx < assignments.size(); ++idx) {
    randomizer.randomize(assignments[idx], 1);
  }
}
}

template <typename ClauseHandle>
auto random_simulation(gate_structure<ClauseHandle> const& structure, uint64_t max_num_rounds)
    -> lit_partitioning<typename clause_traits<ClauseHandle>::lit>
{
  using namespace gatekit::detail;

  using lit_t = typename clause_traits<ClauseHandle>::lit;

  std::size_t const max_var = max_var_index(structure);
  std::vector<std::size_t> const inputs = input_var_indices(structure);

  bitvector_map<> assignments{max_var + 1};
  bitvector_sequence_partition var_partition{max_var + 1};
  bitvector_randomizer randomizer;

  // Randomize all variable assignments to eliminate spurious backbone/equivalence
  // conjectures for variables not occurring in the gate structure. Sorting these
  // out later would diminish the performance of the bitvector_sequence_partition
  // update/compression routines.
  randomize_all(assignments, randomizer);

  uint64_t max_num_bitparallel_rounds =
      (max_num_rounds % 2048 == 0 ? max_num_rounds / 2048 : (max_num_rounds / 2048 + 1));

  for (uint64_t idx = 0; idx < max_num_bitparallel_rounds; ++idx) {
    randomize(assignments, randomizer, inputs, idx);
    propagate_structure(assignments, structure);
    var_partition.add(assignments);
  }

  return var_partition.get_current_partitions<lit_t>();
}

}

#include <gatekit/detail/bitvector_prop.h>

#include <gatekit/detail/bitvector.h>
#include <gatekit/detail/clause_utils.h>
#include <gatekit/gate.h>

#include "../helpers/gate_factory.h"
#include "../helpers/gate_utils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <bitset>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>

namespace gatekit {
namespace detail {

using assignment_spec = std::unordered_map<std::size_t, uint8_t>;

using propagate_structure_test_param = std::tuple<
    std::string,     // description
    assignment_spec, // partial assignment before propagation. Omitted vars are assumed false
    gate_structure<ClauseHandle>, // gate structure to propagate
    assignment_spec               // expected assignment after propagation, may be also partial
    >;


class propagate_structure_tests : public ::testing::TestWithParam<propagate_structure_test_param> {
protected:
  auto get_start_assignment() const -> assignment_spec const& { return std::get<1>(GetParam()); }

  auto get_structure() const -> gate_structure<ClauseHandle> const&
  {
    return std::get<2>(GetParam());
  }

  auto get_expected_assignment() const -> assignment_spec const& { return std::get<3>(GetParam()); }

  auto get_start_assignment_as_bvmap() const -> bitvector_map
  {
    std::size_t const max_var_idx = max_var_index(get_structure());
    bitvector_map result{max_var_idx + 1};

    for (auto iter : get_start_assignment()) {
      if (to_var_index(iter.first) > max_var_idx) {
        throw std::logic_error{"bad test input: start assignment variable index out of range"};
      }

      result[to_var_index(iter.first)].get_words()[0] = iter.second;
    }

    return result;
  }
};

MATCHER_P(bitvector_map_matches, expected_assignment, "")
{
  assignment_spec const& expected = expected_assignment;

  for (auto iter : expected) {
    if (to_var_index(iter.first) > arg.size()) {
      throw std::logic_error{"bad test input: expected assignment variable index out of range"};
    }

    using test_result_ty = assignment_spec::mapped_type;
    test_result_ty const test_result_bits =
        arg[to_var_index(iter.first)].get_words()[0] & std::numeric_limits<test_result_ty>::max();

    if (test_result_bits != iter.second) {
      *result_listener << "var " << iter.first << ": expected=" << std::bitset<8>{iter.second}
                       << ", actual=" << std::bitset<8>{test_result_bits};
      return false;
    }
  }

  return true;
}

TEST_P(propagate_structure_tests, suite)
{
  gate_structure<ClauseHandle> const& structure = get_structure();
  bitvector_map assignment = get_start_assignment_as_bvmap();
  propagate_structure(assignment, structure);
  EXPECT_THAT(assignment, bitvector_map_matches(get_expected_assignment()));
}


namespace {
// binary constants are a C++14 feature, so we're stuck with this for now:
auto b_to_u8(std::string const& bits) -> uint8_t
{
  if (bits.size() != 8) {
    throw std::invalid_argument{"the bits argument must contain exactly 8 chars"};
  }

  uint8_t result = 0;
  for (int i = 7; i >= 0; --i) {
    if (bits[i] == '1') {
      result |= (1 << i);
    }
    else if (bits[i] != '0') {
      throw std::invalid_argument{"the bits argument must contain only 0 and 1"};
    }
  }

  return result;
}
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(propagate_structure_tests, propagate_structure_tests,
  ::testing::Values(
    std::make_tuple("empty structure", assignment_spec{}, to_structure<ClauseHandle>({}, {}), assignment_spec{}),

    std::make_tuple("single and gate (|fwd| < |bwd|), not monotonic, positive output",
      assignment_spec{{1, b_to_u8("10110100")}, {3, b_to_u8("01100101")}},
      to_structure<ClauseHandle>({and_gate({1, 3}, 2)}, {{2}}),
      assignment_spec{{1, b_to_u8("10110100")}, {3, b_to_u8("01100101")}, {2, b_to_u8("00100100")}}),

    std::make_tuple("single and gate (|fwd| < |bwd|), not monotonic, negative output",
      assignment_spec{{1, b_to_u8("10110100")}, {3, b_to_u8("01100101")}},
      to_structure<ClauseHandle>({and_gate({1, 3}, -2)}, {{-2}}),
      assignment_spec{{1, b_to_u8("10110100")}, {3, b_to_u8("01100101")}, {2, b_to_u8("11011011")}}),

    std::make_tuple("single or gate (|fwd| > |bwd|), not monotonic, positive output",
      assignment_spec{{1, b_to_u8("10110100")}, {3, b_to_u8("10011010")}},
      to_structure<ClauseHandle>({or_gate({1, -3}, 2)}, {{2}}),
      assignment_spec{{1, b_to_u8("10110100")}, {3, b_to_u8("10011010")}, {2, b_to_u8("11110101")}}),

    std::make_tuple("single or gate (|fwd| > |bwd|), not monotonic, negative output",
      assignment_spec{{1, b_to_u8("10110100")}, {3, b_to_u8("10011010")}},
      to_structure<ClauseHandle>({or_gate({1, -3}, -2)}, {{-2}}),
      assignment_spec{{1, b_to_u8("10110100")}, {3, b_to_u8("10011010")}, {2, b_to_u8("00001010")}}),

    std::make_tuple("single or gate (|fwd| > |bwd|), monotonic, positive output",
      assignment_spec{{1, b_to_u8("10110100")}, {3, b_to_u8("10011010")}},
      to_structure<ClauseHandle>({monotonic(or_gate({1, -3}, 2))}, {{2}}),
      assignment_spec{{1, b_to_u8("10110100")}, {3, b_to_u8("10011010")}, {2, b_to_u8("11110101")}}),

    std::make_tuple("single or gate (|fwd| > |bwd|), monotonic, negative output",
      assignment_spec{{1, b_to_u8("10110100")}, {3, b_to_u8("10011010")}},
      to_structure<ClauseHandle>({monotonic(or_gate({1, -3}, -2))}, {{-2}}),
      assignment_spec{{1, b_to_u8("10110100")}, {3, b_to_u8("10011010")}, {2, b_to_u8("00001010")}}),

    std::make_tuple("single xor gate (|fwd| = |bwd|), not monotonic, positive output",
      assignment_spec{{1, b_to_u8("11110101")}, {2, b_to_u8("11111010")}},
      to_structure<ClauseHandle>({xor_gate(1, 2, 3)}, {{3}}),
      assignment_spec{{1, b_to_u8("11110101")}, {2, b_to_u8("11111010")}, {3, b_to_u8("00001111")}}),

    std::make_tuple("small gate structure: full adder",
      assignment_spec{{101, b_to_u8("11110101")}, {102, b_to_u8("11011100")}, {103, b_to_u8("01010001")}},
      to_structure<ClauseHandle>({monotonic(xor_gate(10, 103, 1)),
                                  monotonic(or_gate({11, 12}, 2)),
                                    monotonic(and_gate({10, 103}, 11)),
                                    monotonic(and_gate({101, 102}, 12)),
                                    xor_gate(101, 102, 10)
                                 }, {{1}, {2}}),
      assignment_spec{{1, b_to_u8("01111000")}, {2, b_to_u8("11010101")}})
));
// clang-format on
}
}

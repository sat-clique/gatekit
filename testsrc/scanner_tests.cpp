#include <gatekit/scanner.h>

#include "helpers/gate_factory.h"
#include "helpers/gate_utils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cassert>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <vector>

namespace gatekit {
using GateList = std::vector<gate<ClauseHandle>>;

template <typename C>
std::string to_string(gate<C> const& gate);

auto operator==(gate<ClauseHandle> const& lhs, gate<ClauseHandle> const& rhs) -> bool
{
  if (&lhs == &rhs) {
    return true;
  }

  if (!lhs.is_nested_monotonically && !rhs.is_nested_monotonically &&
      lhs.output == detail::negate(rhs.output)) {
    return with_flipped_output_sign(lhs) == rhs;
  }

  if (lhs.output != rhs.output || lhs.num_fwd_clauses != rhs.num_fwd_clauses ||
      lhs.is_nested_monotonically != rhs.is_nested_monotonically) {
    return false;
  }

  if (lhs.inputs.size() != rhs.inputs.size() || lhs.clauses.size() != rhs.clauses.size()) {
    return false;
  }

  if (!std::is_permutation(lhs.inputs.begin(), lhs.inputs.end(), rhs.inputs.begin())) {
    return false;
  }

  if (!std::is_permutation(lhs.clauses.begin(), lhs.clauses.end(), rhs.clauses.begin())) {
    return false;
  }

  return true;
}

auto operator==(gate_structure<ClauseHandle> const& lhs, gate_structure<ClauseHandle> const& rhs)
    -> bool
{
  if (&lhs == &rhs) {
    return true;
  }

  if (lhs.roots.size() != rhs.roots.size() || lhs.gates.size() != rhs.gates.size()) {
    return false;
  }

  if (!std::is_permutation(lhs.roots.begin(), lhs.roots.end(), rhs.roots.begin())) {
    return false;
  }

  if (!std::is_permutation(lhs.gates.begin(), lhs.gates.end(), rhs.gates.begin())) {
    return false;
  }

  return true;
}


auto operator<<(std::ostream& stream, gate_structure<ClauseHandle> const& to_dump) -> std::ostream&
{
  stream << to_string(to_dump);
  return stream;
}


using scanner_test_param = std::tuple<
    std::string,                  // Description
    gate_structure<ClauseHandle>, // Test input. Clauses used as scanner input and expected output.
    ClauseList                    // additional scanner input clauses, should be ignored in output
    >;

class scanner_tests : public ::testing::TestWithParam<scanner_test_param> {
public:
  auto create_clauses() const -> std::vector<ClauseHandle>
  {
    std::vector<ClauseHandle> result;

    for (gate<ClauseHandle> const& gate : get_gates()) {
      result.insert(result.end(), gate.clauses.begin(), gate.clauses.end());
    }

    for (Clause const& clause : get_roots()) {
      result.emplace_back(std::make_shared<Clause>(clause));
    }

    ClauseList const& additional = std::get<2>(GetParam());
    for (Clause const& clause : additional) {
      result.emplace_back(std::make_shared<Clause>(clause));
    }

    return result;
  }

  auto get_gates() const -> GateList const& { return std::get<1>(GetParam()).gates; }

  auto get_roots() const -> std::vector<Clause> const& { return std::get<1>(GetParam()).roots; }

  auto get_expected_gate_structure() const -> gate_structure<ClauseHandle> const&
  {
    return std::get<1>(GetParam());
  }
};

TEST_P(scanner_tests, suite)
{
  auto const& input_clauses = create_clauses();

  gate_structure<ClauseHandle> actual =
      scan_gates<ClauseHandle>(input_clauses.begin(), input_clauses.end());
  gate_structure<ClauseHandle> const& expected = get_expected_gate_structure();

  EXPECT_THAT(actual, ::testing::Eq(expected));
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(scanner_tests, scanner_tests,
  ::testing::Values(
    std::make_tuple("empty", to_structure<ClauseHandle>({}, {}), ClauseList{}),

    std::make_tuple("single gate, monotonically nested, not fully encoded, single root, no side problem",
      to_structure<ClauseHandle>({monotonic(and_gate({2, 3, 4}, 1))}, {{1}}), ClauseList{}),

    std::make_tuple("single gate, monotonically nested, fully encoded, single root, no side problem",
      to_structure<ClauseHandle>({monotonic(and_gate({2, 3, 4}, 1), encoding::full)}, {{1}}), ClauseList{}),

    std::make_tuple("multiple gates, monotonically nested, single root, no side problem, distinct inputs (1)",
      to_structure<ClauseHandle>({
        monotonic(or_gate({-21, 22, 23}, 10)),
          monotonic(and_gate({31, -32}, 22)),
          monotonic(xor_gate(41, 42, 23))
      }, {{10}}),
      ClauseList{}),

    std::make_tuple("multiple gates, monotonically nested, single root, no side problem, distinct inputs (2)",
      to_structure<ClauseHandle>({
        monotonic(or_gate({-21, 22, 23}, 10), encoding::full),
          monotonic(and_gate({31, -32}, 22)),
          monotonic(xor_gate(41, 42, 23))
      }, {{10}}),
      ClauseList{}),

    std::make_tuple("multiple gates, single root, no side problem, distinct inputs (1)",
      to_structure<ClauseHandle>({
        monotonic(or_gate({-21, 22, 23}, 10), encoding::full),
          monotonic(and_gate({31, -32}, 22)),
          monotonic(xor_gate(41, 42, 23)),
            and_gate({51, 52}, 42),
            xor_gate(61, 62, 41)
      }, {{10}}),
      ClauseList{}),

    std::make_tuple("multiple gates, single root, no side problem, distinct inputs (2)",
      to_structure<ClauseHandle>({
        monotonic(xor_gate(21, 22, -10), encoding::full),
          and_gate({31, 32}, 21),
            and_gate({41, 42}, 31)
      }, {{-10}}),
      ClauseList{})
));
// clang-format on
}

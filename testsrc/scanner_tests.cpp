#include <gatekit/scanner.h>

#include "helpers/gate_factory.h"

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

namespace {
auto to_structure(std::vector<gate<ClauseHandle>>&& gates, std::vector<std::vector<int>>&& roots)
    -> gate_structure<ClauseHandle>
{
  gate_structure<ClauseHandle> result;
  result.gates = std::move(gates);
  result.roots = std::move(roots);
  return result;
}
}

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


template <typename Iterable, typename ToStringFn>
std::string iterable_to_string(Iterable const& iterable, ToStringFn&& to_string_fn)
{
  std::string result = "[";

  auto const stop = iterable.end();
  for (auto iter = iterable.begin(); iter != stop; ++iter) {
    result += to_string_fn(*iter);

    if (std::next(iter) != stop) {
      result += ", ";
    }
  }

  result += "]";
  return result;
}

template <typename Iterable>
std::string iterable_to_string(Iterable const& iterable)
{
  using item_type = typename std::decay<decltype(*iterable.begin())>::type;

  return iterable_to_string(iterable, [](item_type const& item) {
    using std::to_string;
    return to_string(item);
  });
}

template <typename C>
std::string to_string(gate<C> const& gate)
{
  using std::to_string;

  std::string result = "{";
  result += "inputs: " + iterable_to_string(gate.inputs) + ", ";
  result += "output: " + to_string(gate.output) + ", ";
  result += "num_fwd_clauses: " + to_string(gate.num_fwd_clauses) + ", ";
  result += "is_nested_monotonically: " + to_string(gate.is_nested_monotonically) + ", ";
  result += "clauses: " + iterable_to_string(gate.clauses, [](C const& clause) {
              return iterable_to_string(::gatekit::detail::iterate(clause));
            });

  result += "}";
  return result;
}

template <typename C>
std::string to_string(gate_structure<C> const& structure)
{
  using lit = typename clause_traits<C>::lit;

  std::string result = "{";
  result += "gates: " + iterable_to_string(structure.gates) + ", ";
  result += "roots: " + iterable_to_string(structure.roots, [](std::vector<lit> const& roots) {
              return iterable_to_string(roots);
            });

  result += "}";
  return result;
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
    std::make_tuple("empty", to_structure({}, {}), ClauseList{}),

    std::make_tuple("single gate, monotonically nested, not fully encoded, single root, no side problem",
      to_structure({monotonic(and_gate({2, 3, 4}, 1))}, {{1}}), ClauseList{}),

    std::make_tuple("single gate, monotonically nested, fully encoded, single root, no side problem",
      to_structure({monotonic(and_gate({2, 3, 4}, 1), encoding::full)}, {{1}}), ClauseList{}),

    std::make_tuple("multiple gates, monotonically nested, single root, no side problem, distinct inputs (1)",
      to_structure({
        monotonic(or_gate({-21, 22, 23}, 10)),
          monotonic(and_gate({31, -32}, 22)),
          monotonic(xor_gate(41, 42, 23))
      }, {{10}}),
      ClauseList{}),

    std::make_tuple("multiple gates, monotonically nested, single root, no side problem, distinct inputs (2)",
      to_structure({
        monotonic(or_gate({-21, 22, 23}, 10), encoding::full),
          monotonic(and_gate({31, -32}, 22)),
          monotonic(xor_gate(41, 42, 23))
      }, {{10}}),
      ClauseList{}),

    std::make_tuple("multiple gates, single root, no side problem, distinct inputs (1)",
      to_structure({
        monotonic(or_gate({-21, 22, 23}, 10), encoding::full),
          monotonic(and_gate({31, -32}, 22)),
          monotonic(xor_gate(41, 42, 23)),
            and_gate({51, 52}, 42),
            xor_gate(61, 62, 41)
      }, {{10}}),
      ClauseList{}),

    std::make_tuple("multiple gates, single root, no side problem, distinct inputs (2)",
      to_structure({
        monotonic(xor_gate(21, 22, -10), encoding::full),
          and_gate({31, 32}, 21),
            and_gate({41, 42}, 31)
      }, {{-10}}),
      ClauseList{})
));
// clang-format on
}

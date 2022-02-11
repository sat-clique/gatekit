#include <gatekit/random_simulation.h>

#include <gatekit/detail/utils.h>
#include <gatekit/gate.h>

#include "helpers/gate_factory.h"
#include "helpers/gate_utils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ostream>
#include <string>
#include <utility>

namespace gatekit {
auto normalize_lit_partitioning(lit_partitioning<int> const& to_normalize) -> lit_partitioning<int>
{
  lit_partitioning<int> result = to_normalize;

  std::sort(result.backbones.begin(), result.backbones.end());
  for (std::vector<int>& equivalences : result.equivalences) {
    // TODO: also normalize signs: ensure that the lowermost literal's sign is
    // positive
    std::sort(equivalences.begin(), equivalences.end());
  }

  return result;
}

auto operator<<(std::ostream& stream, lit_partitioning<int> const& to_print) -> std::ostream&
{
  stream << "backbones: " << detail::iterable_to_string(to_print.backbones) << ", equivalences: "
         << detail::iterable_to_string(to_print.equivalences,
                                       [](std::vector<int> const& equivalence_set) {
                                         return detail::iterable_to_string(equivalence_set);
                                       });

  return stream;
}

MATCHER_P(is_equivalent_partitioning, expected, "")
{
  lit_partitioning<int> actual = normalize_lit_partitioning(arg);
  lit_partitioning<int> expected_copy = normalize_lit_partitioning(expected);

  if (actual.backbones != expected_copy.backbones) {
    *result_listener << "mismatching backbone literals";
    return false;
  }

  if (actual.equivalences.size() != expected_copy.equivalences.size()) {
    *result_listener << "mismatching equivalence class count";
    return false;
  }

  if (!std::is_permutation(actual.equivalences.begin(),
                           actual.equivalences.end(),
                           expected_copy.equivalences.begin())) {
    *result_listener << "mismatching equivalence classes";
    return false;
  }

  return true;
}

using random_simulation_test_param =
    std::tuple<std::string,                  // description
               gate_structure<ClauseHandle>, // gate structure on which to perform random simulation
               lit_partitioning<int>         // expected result
               >;

class random_simulation_tests : public ::testing::TestWithParam<random_simulation_test_param> {
};

TEST_P(random_simulation_tests, suite)
{
  gate_structure<ClauseHandle> const& input = std::get<1>(GetParam());
  lit_partitioning<int> const& expected = std::get<2>(GetParam());

  lit_partitioning<int> result = random_simulation(input, 5000);

  EXPECT_THAT(result, is_equivalent_partitioning(expected));
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(random_simulation_tests, random_simulation_tests,
  ::testing::Values(
    std::make_tuple("empty", to_structure<ClauseHandle>({}, {}), lit_partitioning<int>{}),
    std::make_tuple("lone and gate yields empty result", to_structure<ClauseHandle>({and_gate({1, 2}, 3)}, {{3}}), lit_partitioning<int>{}),
    std::make_tuple("duplicated AND gate yields equivalence conjecture",
      to_structure<ClauseHandle>({
          and_gate({1, 2}, 3),
          and_gate({1, 2}, 4)},
        {{3, 4}}),
      lit_partitioning<int>{{}, {{3, 4}}}),
    std::make_tuple("gate with constantly-negative output yields backbone conjecture",
      to_structure<ClauseHandle>({
          and_gate({10, 20}, 1),
            and_gate({100, 200}, 10),
            or_gate({-100, -200}, 20)},
        {{1}}),
    lit_partitioning<int>{{-1}, {{10, -20}}})
));
// clang-format on
}

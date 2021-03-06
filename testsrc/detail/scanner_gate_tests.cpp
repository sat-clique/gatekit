#include <gatekit/detail/occurrence_list.h>
#include <gatekit/detail/scanner_gate.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <tuple>

namespace gatekit {
namespace detail {

using Clause = std::vector<int>;
using ClauseHandle = Clause const*;
using ClauseList = std::vector<Clause>;

enum class is_gate { yes, no };

using is_gate_output_test_param = std::tuple<std::string, // description
                                             ClauseList,  // clauses in occurrence list
                                             int,         // output literal
                                             bool,        // scan as monotonically nested gate
                                             is_gate      // expected result
                                             >;

class is_gate_output_tests : public ::testing::TestWithParam<is_gate_output_test_param> {
};

TEST_P(is_gate_output_tests, suite)
{
  std::vector<Clause> const& input_clauses = std::get<1>(GetParam());
  int const output = std::get<2>(GetParam());
  bool const is_mono = std::get<3>(GetParam());

  std::vector<Clause const*> handles;
  for (Clause const& clause : input_clauses) {
    handles.push_back(&clause);
  }

  occurrence_list<ClauseHandle> clauses{handles.begin(), handles.end()};

  bool const expected = std::get<4>(GetParam()) == is_gate::yes;
  EXPECT_THAT(is_gate_output(output, clauses, is_mono), ::testing::Eq(expected));
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(is_gate_output_tests, is_gate_output_tests,
  ::testing::Values(
    std::make_tuple("unary literal is not gate", ClauseList{{1}}, 1, true, is_gate::no),
    std::make_tuple("lone AND gate is gate", ClauseList{{1, -2, -3}, {-1, 2}, {-1, 3}}, 1, false, is_gate::yes),
    std::make_tuple("monotonically nested, optimized AND gate is gate (1)", ClauseList{{1, -2, -3}}, -1, true, is_gate::yes),
    std::make_tuple("monotonically nested, optimized AND gate is gate (2)", ClauseList{{-1, 2}, {-1, -3}}, 1, true, is_gate::yes),
    std::make_tuple("monotonically nested, unoptimized AND gate is gate", ClauseList{{1, -2, -3}, {-1, 2}, {-1, 3}}, 1, true, is_gate::yes),
    std::make_tuple("nonmonotonically nested, optimized AND gate is not gate (1)", ClauseList{{-1, 2}, {-1, -3}}, 1, false, is_gate::no),
    std::make_tuple("nonmonotonically nested, optimized AND gate is not gate (2)", ClauseList{{1, -2, -3}}, -1, false, is_gate::no),
    std::make_tuple("lone AND gate minus 1 literal is not gate", ClauseList{{1, -3}, {-1, 2}, {-1, 3}}, 1, false, is_gate::no),
    std::make_tuple("lone AND gate minus 1 clause is not gate", ClauseList{{1, -2, -3}, {-1, 2}}, 1, false, is_gate::no),
    std::make_tuple("lone AND gate with additional output unary is not gate", ClauseList{{1, -2, -3}, {-1, 2}, {-1, 3}, {1}}, 1, false, is_gate::no),
    std::make_tuple("lone AND gate with additional input unary is not gate", ClauseList{{1, -2, -3}, {-1, 2}, {-1, 3}, {-3}}, 1, false, is_gate::yes),
    std::make_tuple("monotonically nested, optimized AND gate with flipped output is not gate", ClauseList{{1, -2, -3}}, 1, true, is_gate::no),

    std::make_tuple("nonmonotonically nested XOR gate is gate", ClauseList{{1, -2, 3}, {-1, 2, 3}, {-1, -2, -3}, {1, 2, -3}}, 3, false, is_gate::yes),
    std::make_tuple("full gate simplified with self-subsuming resolution is gate", ClauseList{{-3, -4, -5}, {4, -5}, {3, -4, 5}}, 5, false, is_gate::yes),
    std::make_tuple("full gate simplified with self-subsuming resolution, but missing a clause is not gate",
      ClauseList{{-3, -4, -5}, {3, -4, 5}}, 5, false, is_gate::no),

    std::make_tuple("ternary at-least-2 gate is gate",
      ClauseList{{-1, -2, 4}, {-1, -3, 4}, {-2, -3, 4},
                 {1, 2, -4}, {1, 3, -4}, {2, 3, -4}},
      4, false, is_gate::yes),

    std::make_tuple("nonmonotonically nested if-then-else gate is gate",
      ClauseList{{-1, -2, 3}, {-1, 2, -3}, {1, -2, 3}, {1, 2, -3}},
      3, false, is_gate::yes),

    std::make_tuple("half a gate is not gate", ClauseList{{1, -2, -3}, {-1, -2, 3}}, 3, false, is_gate::no)
));
// clang-format on
}
}

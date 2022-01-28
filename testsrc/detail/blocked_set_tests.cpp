#include "gmock/gmock.h"

#include "gtest/gtest.h"
#include <gatekit/detail/blocked_set.h>
#include <gatekit/detail/occurrence_list.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <tuple>
#include <vector>

namespace gatekit {
namespace detail {

using Clause = std::vector<int>;
using ClauseHandle = Clause const*;
using ClauseVec = std::vector<Clause>;

using blocked_set_test_param = std::tuple<std::string, // description
                                          ClauseVec,   // clauses
                                          int,         // literal tested for blockedness
                                          bool         // expected blockedness
                                          >;

class blocked_set_tests : public ::testing::TestWithParam<blocked_set_test_param> {
};

TEST_P(blocked_set_tests, test_for_blockedness)
{
  blocked_set_test_param const& test_param = GetParam();

  std::vector<ClauseHandle> clauses;
  for (auto const& clause : std::get<1>(test_param)) {
    clauses.push_back(&clause);
  }

  occurrence_list<ClauseHandle> occurrences{clauses.begin(), clauses.end()};

  EXPECT_THAT(is_blocked(std::get<2>(test_param), occurrences),
              ::testing::Eq(std::get<3>(test_param)));
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(blocked_set_tests, blocked_set_tests,
  ::testing::Values(
    std::make_tuple("empty set is blocked", ClauseVec{}, 1, true),
    std::make_tuple("lone unary is blocked", ClauseVec{{2}}, 2, true),
    std::make_tuple("pure literal in lone binary clause is blocked", ClauseVec{{2, -3}}, 2, true),
    std::make_tuple("pure literal in multiple clauses is blocked", ClauseVec{{2, -3}, {2, 5, 6, -3}}, 2, true),
    std::make_tuple("non-pure literal in non-blocked set is not blocked", ClauseVec{{2, -3}, {-2, 4}}, 2, false),
    std::make_tuple("non-pure literal in blocked set is blocked", ClauseVec{{2, -3}, {-2, 3}}, 2, true)
));
// clang-format on
}
}

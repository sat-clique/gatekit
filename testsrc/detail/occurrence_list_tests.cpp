#include "gmock/gmock.h"

#include <gatekit/detail/occurrence_list.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

using ::testing::Eq;
using ::testing::IsEmpty;
using ::testing::UnorderedElementsAre;

namespace gatekit {
namespace detail {

using Clause = std::vector<int>;
using ClauseHandle = Clause const*;

auto create_occurrence_list(std::vector<ClauseHandle> clauses) -> occurrence_list<ClauseHandle>
{
  return occurrence_list<ClauseHandle>{clauses.begin(), clauses.end()};
}

MATCHER(HasNoUnaries, "")
{
  return arg.get_unaries().empty();
}

TEST(occurrence_list_tests, empty)
{
  auto under_test = create_occurrence_list({});

  EXPECT_THAT(under_test, HasNoUnaries());
  EXPECT_THAT(under_test.get_max_lit_index(), Eq(0));
}

TEST(occurrence_list_tests, single_clause)
{
  Clause const input = {1, -2, 3};
  auto under_test = create_occurrence_list({&input});

  EXPECT_THAT(under_test, HasNoUnaries());

  EXPECT_THAT(under_test[5], IsEmpty());
  EXPECT_THAT(under_test[2], IsEmpty());

  EXPECT_THAT(under_test[1], UnorderedElementsAre(&input));
  EXPECT_THAT(under_test[-2], UnorderedElementsAre(&input));
  EXPECT_THAT(under_test[3], UnorderedElementsAre(&input));

  EXPECT_THAT(under_test.get_max_lit_index(), Eq(5));
}

TEST(occurrence_list_tests, three_clauses)
{
  Clause const input1 = {1, -2, 3};
  Clause const input2 = {2, -1, 5, -10};
  Clause const input3 = {-2, -1, 5};

  auto under_test = create_occurrence_list({&input1, &input2, &input3});

  EXPECT_THAT(under_test, HasNoUnaries());

  EXPECT_THAT(under_test[1], UnorderedElementsAre(&input1));
  EXPECT_THAT(under_test[2], UnorderedElementsAre(&input2));
  EXPECT_THAT(under_test[3], UnorderedElementsAre(&input1));
  EXPECT_THAT(under_test[4], IsEmpty());
  EXPECT_THAT(under_test[5], UnorderedElementsAre(&input2, &input3));
  EXPECT_THAT(under_test[10], IsEmpty());

  EXPECT_THAT(under_test[-1], UnorderedElementsAre(&input2, &input3));
  EXPECT_THAT(under_test[-2], UnorderedElementsAre(&input1, &input3));
  EXPECT_THAT(under_test[-3], IsEmpty());

  EXPECT_THAT(under_test.get_max_lit_index(), Eq(19));
}

TEST(occurrence_list_tests, unaries)
{
  Clause const input1 = {10};
  Clause const input2 = {-20};

  auto under_test = create_occurrence_list({&input1, &input2});

  EXPECT_THAT(under_test.get_unaries(), UnorderedElementsAre(10, -20));
  EXPECT_THAT(under_test[10], UnorderedElementsAre(&input1));
  EXPECT_THAT(under_test[-20], UnorderedElementsAre(&input2));

  EXPECT_THAT(under_test.get_max_lit_index(), Eq(39));
}

TEST(occurrence_list_tests, remove_nonunary_clauses)
{
  Clause const input1 = {1, -2, 3};
  Clause const input2 = {2, -1, 5, -10};
  Clause const input3 = {-2, -1, 5};
  Clause const input4 = {5};

  auto under_test = create_occurrence_list({&input1, &input2, &input3, &input4});

  under_test.remove(&input2);

  EXPECT_THAT(under_test[5], UnorderedElementsAre(&input3, &input4));

  EXPECT_FALSE(under_test.empty());

  under_test.remove(&input1);
  under_test.remove(&input3);
  under_test.remove(&input4);

  EXPECT_TRUE(under_test.empty());
  EXPECT_THAT(under_test.get_unaries(), IsEmpty());
}

TEST(occurrence_list_tests, remove_unary_clauses)
{
  Clause const input1 = {5};
  Clause const input2 = {6};
  Clause const input3 = {-7};

  auto under_test = create_occurrence_list({&input1, &input2, &input3});

  under_test.remove_unary(6);

  EXPECT_THAT(under_test[6], IsEmpty());
  EXPECT_THAT(under_test.get_unaries(), UnorderedElementsAre(5, -7));
  EXPECT_FALSE(under_test.empty());
}

TEST(occurrence_list_tests, unknown_literals_do_not_occur)
{
  auto under_test = create_occurrence_list({});
  EXPECT_THAT(under_test[6], IsEmpty());
}

}
}

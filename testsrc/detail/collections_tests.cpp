#include <gatekit/detail/collections.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::Eq;
using ::testing::IsEmpty;
using ::testing::UnorderedElementsAre;

namespace gatekit {
namespace detail {

TEST(literal_set_tests, empty_after_construction)
{
  literal_set<int> under_test{100};
  EXPECT_TRUE(under_test.empty());
  EXPECT_THAT(under_test.size(), Eq(0));
  EXPECT_THAT(under_test.literals(), IsEmpty());
}

TEST(literal_set_tests, elements_contained_once_after_multiple_adds)
{
  literal_set<int> under_test{100};
  under_test.add(1);
  under_test.add(-5);
  under_test.add(2);
  under_test.add(-5);
  under_test.add(5);

  EXPECT_THAT(under_test.literals(), UnorderedElementsAre(1, 2, 5, -5));
  EXPECT_THAT(under_test.size(), Eq(4));

  EXPECT_TRUE(under_test.contains(1));
  EXPECT_TRUE(under_test.contains(2));
  EXPECT_TRUE(under_test.contains(5));
  EXPECT_TRUE(under_test.contains(-5));

  EXPECT_FALSE(under_test.contains(-2));
}

TEST(literal_set_tests, literals_beyond_max_index_are_not_contained)
{
  literal_set<int> under_test{100};
  EXPECT_FALSE(under_test.contains(-10000));
}
}
}

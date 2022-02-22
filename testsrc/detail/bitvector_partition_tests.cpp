#include <gatekit/detail/bitvector_partition.h>

#include <gatekit/detail/bitvector.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <numeric>
#include <vector>

using ::testing::Eq;
using ::testing::IsEmpty;
using ::testing::UnorderedElementsAreArray;

namespace gatekit {
namespace detail {

namespace {

auto contains_permutation(std::vector<std::vector<int>> const& sets,
                          std::vector<int> const& to_find) -> bool
{
  return sets.end() !=
         std::find_if(sets.begin(), sets.end(), [&to_find](std::vector<int> const& item) {
           if (item.size() != to_find.size()) {
             return false;
           }
           return std::is_permutation(item.begin(), item.end(), to_find.begin());
         });
}

auto negated(std::vector<int>& vec) -> std::vector<int>
{
  std::vector<int> result = vec;

  for (int& item : result) {
    item *= -1;
  }

  return result;
}

auto contains_equivalent_set(std::vector<std::vector<int>> const& sets, std::vector<int> to_find)
    -> bool
{
  return contains_permutation(sets, to_find) || contains_permutation(sets, negated(to_find));
}

MATCHER_P(HasEquivalencies, expected, "")
{
  lit_partitioning<int> const& partition = arg;

  std::vector<std::vector<int>> const& actual = partition.equivalences;
  std::vector<std::vector<int>> const& expected_vec = expected;

  if (expected_vec.size() != actual.size()) {
    *result_listener << ::testing::PrintToString(actual) << ": unexpected number of equivalencies";
    return false;
  }

  for (std::vector<int> const& expected_item : expected_vec) {
    if (!contains_equivalent_set(actual, expected_item)) {
      *result_listener << ::testing::PrintToString(actual)
                       << ": does not contain any set equivalent to "
                       << ::testing::PrintToString(expected_item);
      return false;
    }
  }

  return true;
};

auto create_bitvector_map_with_distinct_signatures() -> bitvector_map
{
  bitvector_map result{8};

  for (std::size_t idx = 0; idx < result.size(); ++idx) {
    result[idx].fill(idx + 1);
  }

  return result;
}
}

TEST(bitvector_partition_tests, initially_all_positive_backbones)
{
  bitvector_sequence_partition under_test{8};
  lit_partitioning<int> result = under_test.get_current_partitions<int>();

  std::vector<int> expected_backbones(8);
  std::iota(expected_backbones.begin(), expected_backbones.end(), 1);

  EXPECT_THAT(result.backbones, UnorderedElementsAreArray(expected_backbones));
  EXPECT_THAT(result.equivalences, IsEmpty());
}

TEST(bitvector_partition_tests, when_all_signatures_are_different_then_partitions_are_empty)
{
  bitvector_sequence_partition under_test{8};

  under_test.add(create_bitvector_map_with_distinct_signatures());

  lit_partitioning<int> result = under_test.get_current_partitions<int>();
  EXPECT_THAT(result.backbones, IsEmpty());
  EXPECT_THAT(result.equivalences, IsEmpty());
}

TEST(bitvector_partition_tests, when_signatures_are_equivalent_partitions_are_created)
{
  bitvector_sequence_partition under_test{8};

  bitvector_map input = create_bitvector_map_with_distinct_signatures();
  input[0].fill(123ull);
  input[1].fill(100ull);
  input[2].fill(~10ull);
  input[3].fill(200ull);
  input[4].fill(~100ull);
  input[6].fill(100ull);
  input[7].fill(200ull);

  under_test.add(input);

  input[0].fill(123ull);
  input[1].fill(101ull);
  input[2].fill(~10ull);
  input[3].fill(201ull);
  input[4].fill(~101ull);
  input[6].fill(101ull);
  input[7].fill(201ull);

  under_test.add(input);

  lit_partitioning<int> result = under_test.get_current_partitions<int>();

  EXPECT_THAT(result.backbones, IsEmpty());
  EXPECT_THAT(result, HasEquivalencies(std::vector<std::vector<int>>{{2, -5, 7}, {4, 8}}));
}

}
}

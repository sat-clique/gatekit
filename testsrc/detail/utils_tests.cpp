#include <gatekit/detail/utils.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <tuple>
#include <vector>

namespace gatekit {
namespace detail {

using unstable_erase_all_tests_param = std::tuple<std::vector<int>, // container from which to erase
                                                  std::vector<int>  // elements to erase
                                                  >;


class erase_functions_tests : public ::testing::TestWithParam<unstable_erase_all_tests_param> {
public:
  void SetUp() override
  {
    m_to_erase_from = std::get<0>(GetParam());
    m_to_erase = std::get<1>(GetParam());

    m_expected_result = m_to_erase_from;
    for (int item : m_to_erase) {
      m_expected_result.erase(std::remove(m_expected_result.begin(), m_expected_result.end(), item),
                              m_expected_result.end());
    }
  }

protected:
  std::vector<int> m_to_erase_from;
  std::vector<int> m_to_erase;
  std::vector<int> m_expected_result;
};

TEST_P(erase_functions_tests, unstable_erase_first_all_few_suite)
{
  unstable_erase_first_all_few(m_to_erase_from, m_to_erase);

  EXPECT_THAT(m_to_erase, ::testing::IsEmpty());
  EXPECT_THAT(m_to_erase_from, ::testing::UnorderedElementsAreArray(m_expected_result));
}

TEST_P(erase_functions_tests, unstable_erase_first_all_many_suite)
{
  unstable_erase_first_all_many(m_to_erase_from, m_to_erase);

  EXPECT_THAT(m_to_erase, ::testing::IsEmpty());
  EXPECT_THAT(m_to_erase_from, ::testing::UnorderedElementsAreArray(m_expected_result));
}

TEST_P(erase_functions_tests, erase_all_hashsorted_suite_default)
{
  using Hash = std::hash<int>;
  auto hash_compare = [](int lhs, int rhs) { return Hash{}(lhs) < Hash{}(rhs); };
  std::sort(m_to_erase.begin(), m_to_erase.end(), hash_compare);
  std::sort(m_to_erase_from.begin(), m_to_erase_from.end(), hash_compare);

  erase_all_hashsorted<int, Hash>(m_to_erase_from, m_to_erase);

  EXPECT_THAT(m_to_erase_from, ::testing::UnorderedElementsAreArray(m_expected_result));
}

TEST_P(erase_functions_tests, erase_all_hashsorted_suite_conflicts)
{
  struct BadHash {
    auto operator()(int value) noexcept -> size_t { return value / 3; }
  };

  auto hash_compare = [](int lhs, int rhs) { return BadHash{}(lhs) < BadHash{}(rhs); };
  std::sort(m_to_erase.begin(), m_to_erase.end(), hash_compare);
  std::sort(m_to_erase_from.begin(), m_to_erase_from.end(), hash_compare);

  erase_all_hashsorted<int, BadHash>(m_to_erase_from, m_to_erase);

  EXPECT_THAT(m_to_erase_from, ::testing::UnorderedElementsAreArray(m_expected_result));
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(erase_functions_tests, erase_functions_tests,
  ::testing::Values(
    std::make_tuple(std::vector<int>{}, std::vector<int>{}),
    std::make_tuple(std::vector<int>{1}, std::vector<int>{}),
    std::make_tuple(std::vector<int>{1, 2}, std::vector<int>{}),
    std::make_tuple(std::vector<int>{1}, std::vector<int>{1}),
    std::make_tuple(std::vector<int>{1, 2, 3}, std::vector<int>{1}),
    std::make_tuple(std::vector<int>{1, 2, 3}, std::vector<int>{2}),
    std::make_tuple(std::vector<int>{1, 2, 3}, std::vector<int>{3}),
    std::make_tuple(std::vector<int>{1, 2, 3}, std::vector<int>{1, 2}),
    std::make_tuple(std::vector<int>{1, 2, 3}, std::vector<int>{2, 3}),
    std::make_tuple(std::vector<int>{1, 2, 3}, std::vector<int>{1, 2, 3}),
    std::make_tuple(std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8}, std::vector<int>{2, 3, 4, 5, 7, 8})
));
// clang-format on

}
}

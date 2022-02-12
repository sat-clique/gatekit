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

TEST_P(erase_functions_tests, erase_all_sorted_suite_default)
{
  std::sort(m_to_erase.begin(), m_to_erase.end());
  std::sort(m_to_erase_from.begin(), m_to_erase_from.end());

  erase_all_sorted<int>(m_to_erase_from, m_to_erase);

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

using n_choose_k_test_param = std::tuple<std::size_t, // n
                                         std::size_t, // k
                                         std::size_t  // expected result
                                         >;

class n_choose_k_tests : public ::testing::TestWithParam<n_choose_k_test_param> {
};

TEST_P(n_choose_k_tests, suite)
{
  std::size_t const n = std::get<0>(GetParam());
  std::size_t const k = std::get<1>(GetParam());
  std::size_t const expected = std::get<2>(GetParam());

  EXPECT_THAT(n_choose_k(n, k), ::testing::Eq(expected));
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(n_choose_k_tests, n_choose_k_tests,
  ::testing::Values(
    std::make_tuple(1, 0, 1),
    std::make_tuple(1, 1, 1),
    std::make_tuple(2, 0, 1),
    std::make_tuple(2, 1, 2),
    std::make_tuple(2, 2, 1),
    std::make_tuple(3, 0, 1),
    std::make_tuple(4, 0, 1),
    std::make_tuple(4, 1, 4),
    std::make_tuple(4, 2, 6),
    std::make_tuple(4, 3, 4),
    std::make_tuple(4, 4, 1),

    std::make_tuple(std::numeric_limits<uint64_t>::max(), 0, 1),
    std::make_tuple(std::numeric_limits<uint64_t>::max(), 1, std::numeric_limits<uint64_t>::max()),
    std::make_tuple(std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::max() - 1, std::numeric_limits<uint64_t>::max()),
    std::make_tuple(std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::max(), 1)
));
// clang-format on

}
}

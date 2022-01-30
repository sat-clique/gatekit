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


class unstable_erase_first_all_tests
  : public ::testing::TestWithParam<unstable_erase_all_tests_param> {
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

TEST_P(unstable_erase_first_all_tests, suite_few)
{
  unstable_erase_first_all_few(m_to_erase_from, m_to_erase);

  EXPECT_THAT(m_to_erase, ::testing::IsEmpty());
  EXPECT_THAT(m_to_erase_from, ::testing::UnorderedElementsAreArray(m_expected_result));
}

TEST_P(unstable_erase_first_all_tests, suite_many)
{
  unstable_erase_first_all_many(m_to_erase_from, m_to_erase);

  EXPECT_THAT(m_to_erase, ::testing::IsEmpty());
  EXPECT_THAT(m_to_erase_from, ::testing::UnorderedElementsAreArray(m_expected_result));
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(unstable_erase_first_all_tests, unstable_erase_first_all_tests,
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
    std::make_tuple(std::vector<int>{1, 2, 3}, std::vector<int>{1, 2, 3})
));
// clang-format on

}
}

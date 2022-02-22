#include <gatekit/detail/bitvector_rand.h>

#include <gatekit/detail/bitvector.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace gatekit {
namespace detail {

auto get_ones_percentage(bitvector const& bv) -> double
{

  int num_ones = 0;
  for (auto const& word : bv.get_words()) {
    for (unsigned int i = 0; i < 64; ++i) {
      num_ones += ((word & (1ull << i)) != 0 ? 1 : 0);
    }
  }

  return static_cast<double>(num_ones) / (bv.get_words().size() * sizeof(bitvector::word_t) * 8);
}

// parameter: randomization bias
class bitvector_randomizer_tests : public ::testing::TestWithParam<uint32_t> {
};

TEST_P(bitvector_randomizer_tests, suite)
{
  bitvector_randomizer randomizer;
  uint32_t const bias = GetParam();

  double avg_ones = 0.0;
  bitvector bv;

  int const num_rounds = 1024;
  for (int i = 0; i < num_rounds; ++i) {
    bv.fill(i);
    randomizer.randomize(bv, bias);
    avg_ones += get_ones_percentage(bv);
  }

  avg_ones /= num_rounds;
  double expected_avg_ones = 1 - std::pow(0.5, bias);

  EXPECT_THAT(avg_ones, ::testing::DoubleNear(expected_avg_ones, std::pow(0.5, bias + 2)));
}

INSTANTIATE_TEST_SUITE_P(bitvector_randomizer_tests,
                         bitvector_randomizer_tests,
                         ::testing::Values(1, 2, 3, 4, 5));

}
}

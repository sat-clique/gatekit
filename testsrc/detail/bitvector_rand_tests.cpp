#include <gatekit/detail/bitvector_rand.h>

#include <gatekit/detail/bitvector.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace gatekit {
namespace detail {

template <std::size_t Bits, std::size_t Alignment>
auto get_ones_percentage(bitvector<Bits, Alignment> const& bv) -> double
{

  int num_ones = 0;
  for (uint64_t const& word : bv.get_words()) {
    for (int i = 0; i < 64; ++i) {
      num_ones += ((word & (1ull << i)) != 0 ? 1 : 0);
    }
  }

  return static_cast<double>(num_ones) / Bits;
}

// parameter: randomization bias
class bitvector_randomizer_tests : public ::testing::TestWithParam<uint32_t> {
};

TEST_P(bitvector_randomizer_tests, suite)
{
  bitvector_randomizer randomizer;
  uint32_t const bias = GetParam();

  double avg_ones = 0.0;
  bitvector<> bv;

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

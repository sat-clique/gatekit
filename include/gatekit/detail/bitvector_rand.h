#pragma once

#include <gatekit/detail/bitvector.h>
#include <gatekit/detail/utils.h>

#include <numeric>
#include <tuple>

namespace gatekit {
namespace detail {

inline void randomize_bv(bitvector& target, uint64_t additional)
{
  for (auto& word : target.get_words()) {
    word = xorshift_star(word + additional);
  }
}

inline void thicken_bv(bitvector& target, uint64_t additional)
{
  for (auto& word : target.get_words()) {
    word |= xorshift_star(word + additional);
  }
}


class bitvector_randomizer {
public:
  void randomize(bitvector& target, uint32_t bias_exponent)
  {
    assert(bias_exponent > 0);

    m_seed = xorshift_star(m_seed);
    randomize_bv(target, m_seed);

    for (uint32_t bias = 1; bias < bias_exponent; ++bias) {
      thicken_bv(target, m_seed);
    }
  }

private:
  uint64_t m_seed = 0xe73526b9;
};

}
}

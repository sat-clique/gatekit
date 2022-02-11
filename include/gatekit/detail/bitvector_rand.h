#pragma once

#include <gatekit/detail/bitvector.h>
#include <gatekit/detail/utils.h>

#include <numeric>
#include <tuple>

namespace gatekit {
namespace detail {

template <std::size_t Bits, std::size_t Alignment>
void randomize_bv(bitvector<Bits, Alignment>& target, uint64_t additional)
{
  for (auto& word : target.get_words()) {
    word = xorshift_star(word + additional);
  }
}

template <std::size_t Bits, std::size_t Alignment>
void thicken_bv(bitvector<Bits, Alignment>& target, uint64_t additional)
{
  for (auto& word : target.get_words()) {
    word |= xorshift_star(word + additional);
  }
}


class bitvector_randomizer {
public:
  template <std::size_t Bits = 2048, std::size_t Alignment = 64>
  void randomize(bitvector<Bits, Alignment>& target, uint32_t bias_exponent)
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

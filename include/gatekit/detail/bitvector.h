#pragma once

#include <gatekit/detail/utils.h>

#include <array>
#include <cassert>
#include <cstdint>

namespace gatekit {
namespace detail {

template <std::size_t Bits = 2048, std::size_t Alignment = 64>
class alignas(Alignment) bitvector {
  static_assert(Bits % 64 == 0, "The Bits parameter must be a multiple of 64");

public:
  using words_t = std::array<uint64_t, Bits / 64>;

  auto operator~() const noexcept -> bitvector
  {
    bitvector result;
    for (std::size_t idx = 0; idx < m_vars.size(); ++idx) {
      result.m_vars[idx] = ~m_vars[idx];
    }
  }

  auto operator|(bitvector const& rhs) const noexcept -> bitvector
  {
    bitvector result;
    for (std::size_t idx = 0; idx < m_vars.size(); ++idx) {
      result.m_vars[idx] = m_vars[idx] | rhs.m_vars[idx];
    }
  }

  auto operator&(bitvector const& rhs) const noexcept -> bitvector
  {
    bitvector result;
    for (std::size_t idx = 0; idx < m_vars.size(); ++idx) {
      result.m_vars[idx] = m_vars[idx] & rhs.m_vars[idx];
    }
  }

  auto operator^(bitvector const& rhs) const noexcept -> bitvector
  {
    bitvector result;
    for (std::size_t idx = 0; idx < m_vars.size(); ++idx) {
      result.m_vars[idx] = m_vars[idx] ^ rhs.m_vars[idx];
    }
  }

  auto operator|=(bitvector const& rhs) noexcept -> bitvector&
  {
    for (std::size_t idx = 0; idx < m_vars.size(); ++idx) {
      m_vars[idx] |= rhs.m_vars[idx];
    }
  }

  auto operator&=(bitvector const& rhs) noexcept -> bitvector&
  {
    for (std::size_t idx = 0; idx < m_vars.size(); ++idx) {
      m_vars[idx] &= rhs.m_vars[idx];
    }
  }

  auto operator^=(bitvector const& rhs) noexcept -> bitvector&
  {
    for (std::size_t idx = 0; idx < m_vars.size(); ++idx) {
      m_vars[idx] ^= rhs.m_vars[idx];
    }
  }

  auto operator==(bitvector const& rhs) const noexcept -> bool
  {
    return (this == &rhs) || (m_vars == rhs.m_vars);
  }

  auto operator!=(bitvector const& rhs) const noexcept -> bool { return !(*this == rhs); }

  auto get_words() const noexcept -> words_t const& { return m_vars; }

  auto get_words() noexcept -> words_t& { return m_vars; }

  static auto ones() noexcept -> bitvector
  {
    bitvector result;
    for (auto& word : result.m_vars) {
      word = ~static_cast<uint64_t>(0);
    }
  }

  static auto zeros() noexcept -> bitvector
  {
    bitvector result;
    for (auto& word : result.m_vars) {
      word = 0ull;
    }
  }

  auto operator=(bitvector const&) noexcept -> bitvector& = default;
  auto operator=(bitvector&&) noexcept -> bitvector& = default;
  bitvector(bitvector const&) noexcept = default;
  bitvector(bitvector&&) noexcept = default;

private:
  bitvector() = default;

  std::array<uint64_t, Bits / 64> m_vars;
};


template <std::size_t Bits = 2048, std::size_t Alignment = 64>
class bitvector_map {
public:
  using bitvector_t = bitvector<Bits, Alignment>;

  bitvector_map(std::size_t size) { m_bitvectors = allocate_aligned<bitvector_t>(size); }

  auto operator[](std::size_t index) noexcept -> bitvector_t&
  {
    assert(index < m_size);
    return m_bitvectors[index];
  }

  auto operator[](std::size_t index) const noexcept -> bitvector_t const&
  {
    assert(index < m_size);
    return m_bitvectors[index];
  }

private:
  unique_aligned_array_ptr<bitvector_t> m_bitvectors;
  std::size_t m_size;
};

}
}
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

  bitvector() { fill(0ull); }

  auto operator~() const noexcept -> bitvector
  {
    bitvector result;
    for (std::size_t idx = 0; idx < m_vars.size(); ++idx) {
      result.m_vars[idx] = ~m_vars[idx];
    }

    return result;
  }

  auto operator|(bitvector const& rhs) const noexcept -> bitvector
  {
    bitvector result;
    for (std::size_t idx = 0; idx < m_vars.size(); ++idx) {
      result.m_vars[idx] = m_vars[idx] | rhs.m_vars[idx];
    }

    return result;
  }

  auto operator&(bitvector const& rhs) const noexcept -> bitvector
  {
    bitvector result;
    for (std::size_t idx = 0; idx < m_vars.size(); ++idx) {
      result.m_vars[idx] = m_vars[idx] & rhs.m_vars[idx];
    }

    return result;
  }

  auto operator^(bitvector const& rhs) const noexcept -> bitvector
  {
    bitvector result;
    for (std::size_t idx = 0; idx < m_vars.size(); ++idx) {
      result.m_vars[idx] = m_vars[idx] ^ rhs.m_vars[idx];
    }

    return result;
  }

  auto operator|=(bitvector const& rhs) noexcept -> bitvector&
  {
    for (std::size_t idx = 0; idx < m_vars.size(); ++idx) {
      m_vars[idx] |= rhs.m_vars[idx];
    }

    return *this;
  }

  auto operator&=(bitvector const& rhs) noexcept -> bitvector&
  {
    for (std::size_t idx = 0; idx < m_vars.size(); ++idx) {
      m_vars[idx] &= rhs.m_vars[idx];
    }

    return *this;
  }

  auto operator^=(bitvector const& rhs) noexcept -> bitvector&
  {
    for (std::size_t idx = 0; idx < m_vars.size(); ++idx) {
      m_vars[idx] ^= rhs.m_vars[idx];
    }

    return *this;
  }

  auto operator==(bitvector const& rhs) const noexcept -> bool
  {
    return (this == &rhs) || (m_vars == rhs.m_vars);
  }

  auto operator!=(bitvector const& rhs) const noexcept -> bool { return !(*this == rhs); }

  auto get_words() const noexcept -> words_t const& { return m_vars; }

  auto get_words() noexcept -> words_t& { return m_vars; }

  auto is_all_zero() const noexcept -> bool
  {
    return std::all_of(m_vars.begin(), m_vars.end(), [](uint64_t x) { return x == 0; });
  }

  auto is_all_one() const noexcept -> bool
  {
    return std::all_of(
        m_vars.begin(), m_vars.end(), [](uint64_t x) { return x == ~static_cast<uint64_t>(0); });
  }

  static auto ones() noexcept -> bitvector { return bitvector{~static_cast<uint64_t>(0)}; }

  static auto zeros() noexcept -> bitvector { return bitvector{0ull}; }

  void fill(uint64_t value) noexcept
  {
    for (auto& word : m_vars) {
      word = value;
    }
  }

  auto operator=(bitvector const&) noexcept -> bitvector& = default;
  auto operator=(bitvector&&) noexcept -> bitvector& = default;
  bitvector(bitvector const&) noexcept = default;
  bitvector(bitvector&&) noexcept = default;

private:
  explicit bitvector(uint64_t value) { fill(value); }

  std::array<uint64_t, Bits / 64> m_vars;
};


template <std::size_t Bits = 2048, std::size_t Alignment = 64>
class bitvector_map {
public:
  using bitvector_t = bitvector<Bits, Alignment>;

  explicit bitvector_map(std::size_t size) : m_size(size)
  {
    m_bitvectors = allocate_aligned<bitvector_t>(size);
  }

  auto operator[](std::size_t index) noexcept -> bitvector_t&
  {
    assert(index < m_size);
    return m_bitvectors.get()[index];
  }

  auto operator[](std::size_t index) const noexcept -> bitvector_t const&
  {
    assert(index < m_size);
    return m_bitvectors.get()[index];
  }

  auto size() const noexcept -> std::size_t { return m_size; }

private:
  unique_aligned_array_ptr<bitvector_t> m_bitvectors;
  std::size_t m_size;
};


class bitvector_hash {
public:
  template <std::size_t Bits = 2048, std::size_t Alignment = 64>
  void add(bitvector<Bits, Alignment> const& bv)
  {
    for (uint64_t word : bv.get_words()) {
      m_hash = xorshift_star(m_hash ^ word);
    }
  }

  auto operator==(bitvector_hash const& rhs) const noexcept -> bool
  {
    return (this == &rhs) || m_hash == rhs.m_hash;
  }

  auto operator!=(bitvector_hash const& rhs) const noexcept -> bool { return !(*this == rhs); }

  auto get_raw() const noexcept -> uint64_t { return m_hash; }

private:
  uint64_t m_hash = 0;
};

}
}

namespace std {
template <>
struct hash<gatekit::detail::bitvector_hash> {
  auto operator()(gatekit::detail::bitvector_hash const& to_hash) const noexcept -> std::size_t
  {
    return to_hash.get_raw();
  }
};
}

#pragma once

#include <gatekit/detail/clause_utils.h>
#include <gatekit/traits.h>

#include <cassert>
#include <cstdint>
#include <vector>

namespace gatekit {
namespace detail {

template <typename Lit>
class literal_set {
public:
  explicit literal_set(std::size_t max_index) { m_is_contained.resize(max_index); }

  void add(Lit literal)
  {
    assert(to_index(literal) <= m_is_contained.size());

    std::size_t index = to_index(literal);
    if (!m_is_contained[index]) {
      m_is_contained[index] = true;
      m_literals.push_back(literal);
    }
  }

  void add_all(std::vector<Lit> literals)
  {
    for (Lit const& literal : literals) {
      add(literal);
    }
  }


  void clear()
  {
    for (Lit const& literal : m_literals) {
      m_is_contained[to_index(literal)] = false;
    }
    m_literals.clear();
  }


  auto literals() const noexcept -> std::vector<Lit> const& { return m_literals; }

  auto contains(Lit const& lit) const noexcept -> bool
  {
    std::size_t const index = to_index(lit);
    return index < m_is_contained.size() && m_is_contained[to_index(lit)];
  }

  auto empty() const noexcept -> bool { return m_literals.empty(); }

  auto size() const noexcept -> std::size_t { return m_literals.size(); }


private:
  std::vector<Lit> m_literals;

  // Note about performance: in most cases (ie. add()), flags in m_is_contained
  // are read before they are written, so std::vector<bool> has no significant
  // overhead here.
  std::vector<bool> m_is_contained;
};

}
}

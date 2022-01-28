#pragma once

#include <gatekit/detail/clause_utils.h>
#include <gatekit/detail/utils.h>

#include <gatekit/traits.h>

#include <cstdint>
#include <vector>

namespace gatekit {
namespace detail {

template <typename ClauseHandle>
class occurrence_list {
public:
  using lit = typename clause_traits<ClauseHandle>::lit;
  using clause = ClauseHandle;


  template <typename ClauseHandleIter>
  occurrence_list(ClauseHandleIter start, ClauseHandleIter stop)
  {
    for (ClauseHandleIter current_clause = start; current_clause != stop; ++current_clause) {
      add_clause(*current_clause);
    }
  }


  auto operator[](lit const& literal) const noexcept -> std::vector<ClauseHandle> const&
  {
    if (to_index(literal) >= m_clauses_by_lit.size()) {
      return m_empty_clauselist;
    }

    return m_clauses_by_lit[to_index(literal)];
  }


  auto get_max_lit_index() const noexcept -> std::size_t
  {
    return m_clauses_by_lit.empty() ? 0 : (m_clauses_by_lit.size() - 1);
  }

  auto get_unaries() const noexcept -> std::vector<lit> const& { return m_unaries; }

  void remove(ClauseHandle clause)
  {
    for (auto literal : iterate(clause)) {
      unstable_erase_first(m_clauses_by_lit[to_index(literal)], clause);
    }

    if (get_size(clause) == 1) {
      unstable_erase_first(m_unaries, get_lit(clause, 0));
    }
  }

  void remove_all(std::vector<ClauseHandle> const& clauses)
  {
    for (ClauseHandle clause : clauses) {
      remove(clause);
    }
  }

  void remove_unary(lit unary)
  {
    unstable_erase_first_if(m_clauses_by_lit[to_index(unary)],
                            [](ClauseHandle handle) { return get_size(handle) == 1; });
    unstable_erase_first(m_unaries, unary);
  }

  auto empty() const noexcept -> bool
  {
    for (auto const& clauselist : m_clauses_by_lit) {
      if (!clauselist.empty()) {
        return false;
      }
    }

    return true;
  }

private:
  void add_clause(ClauseHandle clause)
  {
    for (lit literal : iterate(clause)) {
      std::size_t const lit_index = to_index(literal);

      if (m_clauses_by_lit.size() <= lit_index) {
        m_clauses_by_lit.resize(lit_index + 1);
      }

      m_clauses_by_lit[lit_index].push_back(&*clause);
    }

    if (get_size(clause) == 1) {
      m_unaries.push_back(get_lit(clause, 0));
    }
  }


  std::vector<std::vector<ClauseHandle>> m_clauses_by_lit;
  std::vector<ClauseHandle> m_empty_clauselist;
  std::vector<lit> m_unaries;
};

}
}

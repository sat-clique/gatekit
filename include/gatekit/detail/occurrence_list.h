#pragma once

#include <gatekit/detail/clause_utils.h>
#include <gatekit/detail/collections.h>
#include <gatekit/detail/utils.h>

#include <gatekit/traits.h>

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <vector>

#include <iostream>
#include <unordered_set>

namespace gatekit {
namespace detail {

template <typename ClauseHandle>
class occurrence_list {
public:
  using lit = typename clause_traits<ClauseHandle>::lit;
  using clause_handle = ClauseHandle;
  using occurrence_vec = std::vector<ClauseHandle>;

  template <typename ClauseHandleIter>
  occurrence_list(ClauseHandleIter start, ClauseHandleIter stop)
  {
    for (ClauseHandleIter current_clause = start; current_clause != stop; ++current_clause) {
      add_clause(*current_clause);
    }
  }


  auto operator[](lit const& literal) const -> std::vector<ClauseHandle> const&
  {
    if (to_index(literal) >= m_clauses_by_lit.size()) {
      return m_empty_clauselist;
    }

    // In most cases, clients will traverse the returned vector, actually
    // removing removed clauses here much more cache-friendly than removing
    // them eagerly.
    erase_clauses_to_remove(to_index(literal));

    return m_clauses_by_lit[to_index(literal)];
  }


  auto get_max_lit_index() const noexcept -> std::size_t
  {
    return m_clauses_by_lit.empty() ? 0 : (m_clauses_by_lit.size() - 1);
  }

  auto get_unaries() const noexcept -> std::vector<lit> const& { return m_unaries; }

  void remove(ClauseHandle clause)
  {
    // m_clauses_by_lit is not updated eagerly: since the occurrence list can
    // grow very large in realistic use cases (>50 million clauses), this is
    // too costly, in particular because it clobbers the cache and it is likely
    // that further literals need to be removed from m_clauses_by_lit[x] before
    // x is queried again. Therefore, m_clauses_by_lit is actually modified in
    // operator[].
    //
    // In large SAT problem instances (for example 13pipe_k, GBD hash
    // 772102b16ea3acaf7b516714b146b6ca), this optimization nearly doubles
    // the speed of scan_gates().

    for (auto literal : iterate(clause)) {
      if (!m_clauses_by_lit[to_index(literal)].empty()) {
        m_clauses_to_remove[to_index(literal)].push_back(clause);
      }
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

  void remove_gate_root(lit output)
  {
    occurrence_vec fwd = std::move(m_clauses_by_lit[to_index(output)]);
    occurrence_vec bwd = std::move(m_clauses_by_lit[to_index(negate(output))]);

    m_clauses_by_lit[to_index(output)] = occurrence_vec{};
    m_clauses_by_lit[to_index(negate(output))] = occurrence_vec{};

    remove_all(fwd);
    remove_all(bwd);

    m_clauses_to_remove[to_index(output)].clear();
    m_clauses_to_remove[to_index(negate(output))].clear();
  }

  void remove_unary(lit unary)
  {
    unstable_erase_first_if(m_clauses_by_lit[to_index(unary)],
                            [](ClauseHandle const& cl) { return get_size(cl) == 1; });
    unstable_erase_first(m_unaries, unary);
  }

  auto empty() const -> bool
  {
    // This function is only used for testing ~> not optimized
    for (size_t index = 0; index < m_clauses_by_lit.size(); ++index) {
      erase_clauses_to_remove(index);
    }

    for (auto const& clauselist : m_clauses_by_lit) {
      if (!clauselist.empty()) {
        return false;
      }
    }

    return true;
  }

  auto get_estimated_lookup_cost(lit literal) const noexcept -> std::size_t
  {
    return m_clauses_to_remove[to_index(literal)].size() +
           m_clauses_to_remove[to_index(negate(literal))].size();
  }

private:
  void add_clause(ClauseHandle clause)
  {
    for (lit literal : iterate(clause)) {
      std::size_t const lit_index = to_index(literal);

      if (m_clauses_by_lit.size() <= lit_index) {
        m_clauses_by_lit.resize(lit_index + 1);
        m_clauses_to_remove.resize(lit_index + 1);
      }

      m_clauses_by_lit[lit_index].emplace_back(clause);
    }

    if (get_size(clause) == 1) {
      m_unaries.push_back(get_lit(clause, 0));
    }
  }

  void erase_clauses_to_remove(size_t index) const
  {
    unstable_erase_first_all(m_clauses_by_lit[index], m_clauses_to_remove[index]);
  }

  // As an optimization, m_clauses_by_lit and m_clauses_to_remove are
  // lazily updated in operator[], which is const (since it does
  // not change observable state), so these members need to be
  // mutable.
  mutable std::vector<std::vector<ClauseHandle>> m_clauses_by_lit;
  mutable std::vector<std::vector<ClauseHandle>> m_clauses_to_remove;

  std::vector<ClauseHandle> m_empty_clauselist;
  std::vector<lit> m_unaries;
};

}
}

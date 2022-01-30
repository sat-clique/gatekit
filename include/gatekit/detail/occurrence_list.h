#pragma once

#include <gatekit/detail/clause_utils.h>
#include <gatekit/detail/collections.h>
#include <gatekit/detail/utils.h>

#include <gatekit/traits.h>

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <vector>

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
    reserve_occ_list_memory(start, stop);

    for (ClauseHandleIter current_clause = start; current_clause != stop; ++current_clause) {
      add_clause(*current_clause);
    }
  }


  auto operator[](lit const& literal) const -> std::vector<ClauseHandle> const&
  {
    if (to_index(literal) >= m_occ_lists_by_lit.size()) {
      return m_empty_clauselist;
    }

    // In most cases, clients will traverse the returned vector, actually
    // removing removed clauses here much more cache-friendly than removing
    // them eagerly.
    erase_clauses_to_remove(to_index(literal));

    return m_occ_lists_by_lit[to_index(literal)].clauses;
  }


  auto get_max_lit_index() const noexcept -> std::size_t
  {
    return m_occ_lists_by_lit.empty() ? 0 : (m_occ_lists_by_lit.size() - 1);
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
      auto& lit_occ_list = m_occ_lists_by_lit[to_index(literal)];
      if (!lit_occ_list.clauses.empty()) {
        lit_occ_list.clauses_to_remove.push_back(clause);
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
    occ_list& fwd_occlist = m_occ_lists_by_lit[to_index(output)];
    occ_list& bwd_occlist = m_occ_lists_by_lit[to_index(negate(output))];

    occurrence_vec fwd = std::move(fwd_occlist.clauses);
    occurrence_vec bwd = std::move(bwd_occlist.clauses);

    fwd_occlist.clauses = occurrence_vec{};
    bwd_occlist.clauses = occurrence_vec{};

    remove_all(fwd);
    remove_all(bwd);

    fwd_occlist.clauses_to_remove.clear();
    bwd_occlist.clauses_to_remove.clear();
  }

  void remove_unary(lit unary)
  {
    unstable_erase_first_if(m_occ_lists_by_lit[to_index(unary)].clauses,
                            [](ClauseHandle const& cl) { return get_size(cl) == 1; });
    unstable_erase_first(m_unaries, unary);
  }

  auto empty() const -> bool
  {
    // This function is only used for testing ~> not optimized
    for (size_t index = 0; index < m_occ_lists_by_lit.size(); ++index) {
      erase_clauses_to_remove(index);
    }

    for (auto const& occlist : m_occ_lists_by_lit) {
      if (!occlist.clauses.empty()) {
        return false;
      }
    }

    return true;
  }

  auto get_estimated_lookup_cost(lit literal) const noexcept -> std::size_t
  {
    return m_occ_lists_by_lit[to_index(literal)].clauses_to_remove.size() +
           m_occ_lists_by_lit[to_index(negate(literal))].clauses_to_remove.size();
  }

private:
  template <typename ClauseHandleIter>
  auto get_occurrence_counts(ClauseHandleIter start, ClauseHandleIter stop)
      -> std::vector<std::size_t>
  {
    std::vector<size_t> num_occurrences;

    for (ClauseHandleIter clause = start; clause != stop; ++clause) {
      for (lit literal : iterate(*clause)) {
        if (num_occurrences.size() <= to_index(literal)) {
          num_occurrences.resize(to_index(literal) + 1);
        }

        ++num_occurrences[to_index(literal)];
      }
    }

    return num_occurrences;
  }

  template <typename ClauseHandleIter>
  void reserve_occ_list_memory(ClauseHandleIter start, ClauseHandleIter stop)
  {
    std::vector<std::size_t> num_occurrences = get_occurrence_counts(start, stop);

    m_occ_lists_by_lit.resize(num_occurrences.size());

    for (std::size_t index = 0; index < num_occurrences.size(); ++index) {
      m_occ_lists_by_lit[index].clauses.reserve(num_occurrences[index]);
    }
  }

  void add_clause(ClauseHandle clause)
  {
    for (lit literal : iterate(clause)) {
      std::size_t const lit_index = to_index(literal);

      if (m_occ_lists_by_lit.size() <= lit_index) {
        m_occ_lists_by_lit.resize(lit_index + 1);
      }

      m_occ_lists_by_lit[lit_index].clauses.emplace_back(clause);
    }

    if (get_size(clause) == 1) {
      m_unaries.push_back(get_lit(clause, 0));
    }
  }

  using Hash = std::hash<ClauseHandle>;

  static void sort_by_hash(std::vector<ClauseHandle>& vec)
  {
    Hash hasher;
    std::sort(vec.begin(), vec.end(), [&hasher](ClauseHandle const& lhs, ClauseHandle const& rhs) {
      return hasher(lhs) < hasher(rhs);
    });
  }

  void erase_clauses_to_remove(size_t index) const
  {
    // In large SAT problem instances (for example 13pipe_k, GBD hash
    // 772102b16ea3acaf7b516714b146b6ca), it is crucial to use a nearly-linear
    // erasing algorithm. Using erase_all_hashsorted() instead of
    // traversing the list and performing linear lookups in
    // the list of clauses to be deleted doubled the speed of scan_gates().

    occ_list& to_update = m_occ_lists_by_lit[index];

    if (to_update.clauses_to_remove.empty()) {
      return;
    }

    if (!to_update.is_sorted) {
      sort_by_hash(to_update.clauses);
      to_update.is_sorted = true;
    }

    sort_by_hash(to_update.clauses_to_remove);

    erase_all_hashsorted<ClauseHandle, Hash>(to_update.clauses, to_update.clauses_to_remove);
    to_update.clauses_to_remove.clear();
  }

  struct occ_list {
    std::vector<ClauseHandle> clauses;           // all clauses in which a given literal occurs
    std::vector<ClauseHandle> clauses_to_remove; // to be lazily removed from clauses
    bool is_sorted = false;                      // true if `clauses` is currently sorted by hash
  };

  // As an optimization, `clauses_to_remove` are lazily removed from
  // `clauses` in operator[], which is const (since it does
  // not change observable state), so m_occ_lists_by_lit needs to be
  // mutable.
  mutable std::vector<occ_list> m_occ_lists_by_lit;

  std::vector<ClauseHandle> m_empty_clauselist;
  std::vector<lit> m_unaries;
};

}
}

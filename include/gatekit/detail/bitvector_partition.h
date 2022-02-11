#pragma once

#include <gatekit/detail/bitvector.h>
#include <gatekit/detail/clause_utils.h>
#include <gatekit/detail/utils.h>
#include <gatekit/gate.h>

#include <cassert>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace gatekit {

template <typename Lit>
struct lit_partitioning {
  std::vector<Lit> backbones;
  std::vector<std::vector<Lit>> equivalences;
};

namespace detail {

class bitvector_sequence_partition {
public:
  explicit bitvector_sequence_partition(std::size_t size) : m_hashes(size)
  {
    for (std::size_t idx = 0; idx < m_hashes.size(); ++idx) {
      m_hashes[idx].index = idx;
    }
  }

  template <std::size_t Bits, std::size_t Alignment>
  void add(bitvector_map<Bits, Alignment> const& bv_map)
  {
    assert(bv_map.size() == m_hashes.size());

    using bitvector_t = bitvector<Bits, Alignment>;

    for (hash_entry& current : m_hashes) {
      bitvector_t const& bv = bv_map[current.index];
      current.pos_hash.add(bv);
      current.neg_hash.add(~bv);
      current.stuck_positive &= bv.is_all_one();
      current.stuck_negative &= bv.is_all_zero();
    }
  }

  void compress()
  {
    std::unordered_map<bitvector_hash, std::size_t> hash_counters;

    for (hash_entry const& entry : m_hashes) {
      ++hash_counters[entry.pos_hash];
      ++hash_counters[entry.neg_hash];
    }

    erase_remove_if(m_hashes, [&hash_counters](hash_entry const& removal_candidate) {
      if (removal_candidate.stuck_negative || removal_candidate.stuck_positive) {
        // stuck-at-fault/backbone candidates are always kept
        return false;
      }

      // Note that removal_candidate.pos_hash and removal_candidate.neg_hash have
      // the same number of occurrences in hash_counters unless a
      // hash collision occurred during the last add() call. That happens extremely
      // rarely, though, and if it happens, producing a faulty partitioning is
      // acceptable. Therefore, checking only pos_hash is sufficient here:
      return hash_counters[removal_candidate.pos_hash] == 1;
    });
  }

  template <typename Lit>
  auto get_current_partitions() -> lit_partitioning<Lit>
  {
    compress();

    lit_partitioning<Lit> result;
    std::unordered_map<bitvector_hash, std::vector<Lit>> equivalences;
    using map_iter = typename decltype(equivalences)::iterator;

    for (hash_entry const& entry : m_hashes) {
      if (entry.stuck_negative || entry.stuck_positive) {
        result.backbones.push_back(to_lit<Lit>(entry.index, entry.stuck_positive));
      }
      else {
        map_iter pos_iter, neg_iter;

        if ((pos_iter = equivalences.find(entry.pos_hash)) != equivalences.end()) {
          pos_iter->second.push_back(to_lit<Lit>(entry.index, true));
        }
        else if ((neg_iter = equivalences.find(entry.neg_hash)) != equivalences.end()) {
          neg_iter->second.push_back(to_lit<Lit>(entry.index, false));
        }
        else {
          equivalences[entry.pos_hash].push_back(to_lit<Lit>(entry.index, true));
        }
      }
    }

    for (hash_entry const& entry : m_hashes) {
      map_iter iter = equivalences.find(entry.pos_hash);
      if (iter != equivalences.end()) {
        result.equivalences.push_back(std::move(iter->second));
        equivalences.erase(iter);
      }
    }

    return result;
  }


private:
  struct hash_entry {
    std::size_t index = 0;
    bitvector_hash pos_hash;
    bitvector_hash neg_hash;
    bool stuck_positive = true;
    bool stuck_negative = true;
  };

  std::vector<hash_entry> m_hashes;
};

}
}

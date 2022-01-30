#pragma once

#include <algorithm>
#include <unordered_set>
#include <vector>

namespace gatekit {
namespace detail {

template <typename T, typename V>
void unstable_erase_first(std::vector<T>& container, V&& value)
{
  auto const stop = container.end();
  for (auto iter = container.begin(); iter != stop; ++iter) {
    if (*iter == value) {
      std::swap(*iter, container.back());
      container.pop_back();
      return;
    }
  }
}


template <typename T, typename Pred>
void unstable_erase_first_if(std::vector<T>& container, Pred&& predicate)
{
  auto const stop = container.end();
  for (auto iter = container.begin(); iter != stop; ++iter) {
    if (predicate(*iter)) {
      std::swap(*iter, container.back());
      container.pop_back();
      return;
    }
  }
}


template <typename T>
void unstable_erase_first_all_few(std::vector<T>& container, std::vector<T>& to_erase)
{
  auto fwd_cursor = container.begin();
  auto bwd_cursor = container.begin() + container.size();

  while (fwd_cursor != bwd_cursor) {
    while (fwd_cursor != bwd_cursor) {
      auto const& element = *(bwd_cursor - 1);
      if (std::find(to_erase.begin(), to_erase.end(), element) == to_erase.end()) {
        break;
      }

      --bwd_cursor;
    }

    while (fwd_cursor != bwd_cursor) {
      auto const& element = *fwd_cursor;
      auto del_iter = std::find(to_erase.begin(), to_erase.end(), element);

      if (del_iter != to_erase.end()) {
        --bwd_cursor;
        std::swap(*fwd_cursor, *bwd_cursor);
        *del_iter = to_erase.back();
        to_erase.pop_back();
        break;
      }

      ++fwd_cursor;
    }
  }

  container.erase(bwd_cursor, container.end());
  to_erase.clear();
}

template <typename T>
void unstable_erase_first_all_many(std::vector<T>& container, std::vector<T>& to_erase)
{
  auto fwd_cursor = container.begin();
  auto bwd_cursor = container.begin() + container.size();

  std::unordered_set<T> to_erase_set{to_erase.begin(), to_erase.end()};

  while (fwd_cursor != bwd_cursor && !to_erase_set.empty()) {
    while (fwd_cursor != bwd_cursor) {
      auto const& element = *(bwd_cursor - 1);
      if (to_erase_set.find(element) == to_erase_set.end()) {
        break;
      }

      --bwd_cursor;
    }

    while (fwd_cursor != bwd_cursor) {
      auto const& element = *fwd_cursor;
      auto del_iter = to_erase_set.find(element);

      if (del_iter != to_erase_set.end()) {
        --bwd_cursor;
        std::swap(*fwd_cursor, *bwd_cursor);
        to_erase_set.erase(del_iter);
        break;
      }

      ++fwd_cursor;
    }
  }

  container.erase(bwd_cursor, container.end());
  to_erase.clear();
}

template <typename T>
void unstable_erase_first_all(std::vector<T>& container, std::vector<T>& to_erase)
{
  if (to_erase.empty() || container.empty()) {
    return;
  }

  if (to_erase.size() < 150) {
    unstable_erase_first_all_few(container, to_erase);
  }
  else {
    unstable_erase_first_all_many(container, to_erase);
  }
}

}
}

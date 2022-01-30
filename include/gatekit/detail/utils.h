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

template <typename It, typename T, typename Hash = std::hash<T>>
auto contained_in_elements_with_same_hash(It start, It stop, T const& element, Hash& hasher) -> bool
{
  std::size_t const element_hash = hasher(element);

  for (It iter = start; iter != stop && hasher(*iter) == element_hash; ++iter) {
    T const& element_in_range = *iter;
    if (element_in_range == element) {
      return true;
    }
  }

  return false;
}

template <typename T, typename Hash = std::hash<T>>
void erase_all_hashsorted(std::vector<T>& container, std::vector<T> const& to_erase)
{
  auto container_cursor = container.begin();
  auto container_new_end = container.begin();
  auto container_stop = container.end();

  auto to_erase_cursor = to_erase.begin();
  auto to_erase_stop = to_erase.end();

  Hash hasher{};

  while (container_cursor != container_stop && to_erase_cursor != to_erase_stop) {
    T const& container_element = *container_cursor;

    bool keep = !contained_in_elements_with_same_hash(
        to_erase_cursor, to_erase_stop, container_element, hasher);

    if (keep) {
      *container_new_end = container_element;
      ++container_new_end;
    }

    ++container_cursor;

    // Skip all values in to_erase which have a smaller hash than *container_cursor.
    // Due to sortedness, they cannot be contained at container_cursor or
    // later.
    if (container_cursor != container_stop) {
      std::size_t const container_cursor_hash = hasher(*container_cursor);
      to_erase_cursor = std::find_if(
          to_erase_cursor, to_erase_stop, [&hasher, container_cursor_hash](T const& item) {
            return hasher(item) >= container_cursor_hash;
          });
    }
  }

  // The remaining elements in container have a hash value greater than any
  // element in to_erase ~> keep them all
  while (container_cursor != container_stop) {
    *container_new_end = *container_cursor;
    ++container_new_end;
    ++container_cursor;
  }

  container.erase(container_new_end, container.end());
}

}
}

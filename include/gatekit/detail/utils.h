#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
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

template <typename T>
void erase_all_sorted(std::vector<T>& container, std::vector<T> const& to_erase)
{
  auto container_cursor = container.begin();
  auto container_new_end = container.begin();
  auto container_stop = container.end();

  auto to_erase_cursor = to_erase.begin();
  auto to_erase_stop = to_erase.end();

  while (container_cursor != container_stop && to_erase_cursor != to_erase_stop) {
    T const& container_element = *container_cursor;

    if (container_element != *to_erase_cursor) {
      *container_new_end = container_element;
      ++container_new_end;
    }
    else {
      ++to_erase_cursor;
    }

    ++container_cursor;
  }

  while (container_cursor != container_stop) {
    *container_new_end = *container_cursor;
    ++container_new_end;
    ++container_cursor;
  }

  container.erase(container_new_end, container.end());
}


template <typename Iterable, typename ToStringFn>
std::string iterable_to_string(Iterable const& iterable, ToStringFn&& to_string_fn)
{
  std::string result = "[";

  auto const stop = iterable.end();
  for (auto iter = iterable.begin(); iter != stop; ++iter) {
    result += to_string_fn(*iter);

    if (std::next(iter) != stop) {
      result += ", ";
    }
  }

  result += "]";
  return result;
}


template <typename Iterable>
std::string iterable_to_string(Iterable const& iterable)
{
  using item_type = typename std::decay<decltype(*iterable.begin())>::type;

  return iterable_to_string(iterable, [](item_type const& item) {
    using std::to_string;
    return to_string(item);
  });
}


template <typename T>
using unique_aligned_array_ptr = std::unique_ptr<T, std::function<void(T*)>>;

template <typename T>
auto allocate_aligned(std::size_t num_objs) -> unique_aligned_array_ptr<T>
{
  uintptr_t const alignment = alignof(T);

  char* const raw_mem = new char[sizeof(T) * num_objs + alignment];
  uintptr_t const offset = alignment - (reinterpret_cast<uintptr_t>(raw_mem) % alignment);
  T* aligned_mem = reinterpret_cast<T*>(raw_mem + offset);

  for (std::size_t i = 0; i < num_objs; ++i) {
    new (aligned_mem + i) T{};
  }

  return unique_aligned_array_ptr<T>{aligned_mem, [raw_mem, num_objs](T* to_dealloc) {
                                       for (std::size_t i = 0; i < num_objs; ++i) {
                                         to_dealloc[i].~T();
                                       }

                                       delete[](raw_mem);
                                     }};
}
}
}

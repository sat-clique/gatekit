#pragma once

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


}
}

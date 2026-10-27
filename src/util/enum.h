#pragma once

#include <cstddef>
#include <type_traits>

/** Position of an enumerator, for enum-indexed std::arrays. */
template <class E>
constexpr std::size_t idx(E e) {
  return static_cast<std::size_t>(static_cast<std::underlying_type_t<E>>(e));
}

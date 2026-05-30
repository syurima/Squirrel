#pragma once

#include <string>
#include <vector>

namespace mutator_common {

inline std::string pick_random_string(
    const std::vector<std::string> &primary_library,
    const std::vector<std::string> &common_library) {
  const unsigned common_size = common_library.size();
  const unsigned library_size = primary_library.size();
  const unsigned double_library_size = library_size * 2;

  const unsigned rand_int = get_rand_int(double_library_size + common_size);
  if (rand_int < double_library_size) {
    return primary_library[rand_int >> 1];
  }

  return common_library[rand_int - double_library_size];
}

template <typename T>
inline T pick_random_element(const std::vector<T> &values) {
  return values[get_rand_int(values.size())];
}

template <typename T>
inline T pick_random_or(const std::vector<T> &values, const T &fallback) {
  if (values.empty()) return fallback;
  return pick_random_element(values);
}

}
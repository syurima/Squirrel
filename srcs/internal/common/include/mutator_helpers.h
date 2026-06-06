#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <cstdint>
#include <chrono>

namespace mutator_common {

// RNG wrapper (seedable) — prefer this over rand().
inline std::mt19937 &get_global_rng() {
  static thread_local std::mt19937 rng((std::random_device())());
  return rng;
}

inline void seed_rng(uint32_t s) { get_global_rng().seed(s); }

inline unsigned get_rand_int(unsigned range) {
  if (range == 0) return 0;
  std::uniform_int_distribution<unsigned> dist(0, range - 1);
  return dist(get_global_rng());
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

inline bool replace_in_vector(const std::string &old_str,
                              const std::string &new_str,
                              std::vector<std::string> &victim) {
  for (size_t i = 0; i < victim.size(); ++i) {
    if (victim[i] == old_str) {
      victim[i] = new_str;
      return true;
    }
  }
  return false;
}

inline bool remove_in_vector(const std::string &str_to_remove,
                             std::vector<std::string> &victim) {
  auto it = std::find(victim.begin(), victim.end(), str_to_remove);
  if (it != victim.end()) {
    victim.erase(it);
    return true;
  }
  return false;
}

}
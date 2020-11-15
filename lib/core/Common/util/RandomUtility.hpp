/* RandomUtility.hpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_UTIL_RANDOM_UTIL_HPP
#define TD_UTIL_RANDOM_UTIL_HPP

#include <algorithm>
#include <random>
#include <vector>

namespace Randomize {
class TDRandomEngine {
public:
    static std::default_random_engine& get()
    {
        static TDRandomEngine instance;
        return instance.eng;
    }

    TDRandomEngine(TDRandomEngine const&) = delete;
    void operator=(TDRandomEngine const&) = delete;
private:
    TDRandomEngine() {
#ifdef RANDOM_SEED_TESTING
      // kind of unfortunate, but this is how we avoid nondeterminism at testing time
      eng = std::default_random_engine {RANDOM_SEED_TESTING};
#else
      std::random_device rdev{};
      eng = std::default_random_engine {rdev()};
#endif
    }
    std::default_random_engine eng;
};

class GaussianRoller {
public:
  GaussianRoller(const int mean, const int variance) : dist(mean, variance) {}

  void set_roller(int mean, int variance) {
    std::normal_distribution<>::param_type new_params{
        static_cast<double>(mean), static_cast<double>(variance)};
    dist.param(new_params);
  }

  inline int roll_tower_tier() { return dist(TDRandomEngine::get()); }

private:
  std::normal_distribution<> dist;
};

class UniformRoller {
public:
  UniformRoller() {}

  // returns values between [0, weight)
  inline double get_roll(const double weight) {
    return std::generate_canonical<double, std::numeric_limits<double>::digits>(
               TDRandomEngine::get()) *
           weight;
  }
};

} // namespace Randomize

#endif

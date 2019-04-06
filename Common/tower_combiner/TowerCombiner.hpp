/* TowerCombiner.hpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_TOWER_COMBINER_HPP
#define TD_TOWER_COMBINER_HPP

#include "factory.hpp"
#include "util/TowerModifiers.hpp"

#include <cstdint>
#include <unordered_map>

#include <map>
#include <string>

class TowerCombiner {
public:
  using modifier_factory_key_t = uint32_t;
  using modifier_factory_value_t = tower_attribute_modifier;
  using modifier_factory_generator_t =
      std::function<modifier_factory_value_t *(float)>;

  TowerCombiner(const std::string &dictionary_fpath,
                const std::string &attribute_cfgfpath);

  bool check_combination(const std::string &word) const;
  tower_properties make_wordcombination(const std::string &word) const;

private:
  const std::string dictionary_filename;
  // const std::string config_file {"default_attribute_values.yaml"};
  const std::string attributecfg_filename;

  const static std::unordered_map<char, uint32_t> character_attribute_map;

  using modifier_factory_t =
      Factory<modifier_factory_value_t, modifier_factory_key_t,
              modifier_factory_generator_t>;
  modifier_factory_t attribute_fact;

  using dictionary_map_t = std::map<std::string, void *>;
  dictionary_map_t dict;
};

#endif

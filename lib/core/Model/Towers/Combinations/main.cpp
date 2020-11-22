/* main.cpp -- part of the DietyTD testing for the tower combinations (this will
 * move shortly...)
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <algorithm>
#include <chrono>
#include <map>
#include <string>
#include <unordered_map>

#include <fstream>
#include <iomanip>
#include <iostream>

#include "TowerCombiner.hpp"

int main(int argc, char *argv[]) {
  const std::string config_file{"/resources/default_attribute_values.yaml"};
  const std::string dict_file{"/resources/word_list.txt"};

  TowerCombiner tower_combiner(dict_file, config_file);

  /*
  using attribute_mapping_t = attribute_mapping<tower_attribute_modifier>;
  std::cout << "attribute mappings:" << std::endl;
  for (auto attrib_it : attribute_mapping_t::character_attribute_map) {
      std::cout << "Key: " << attrib_it.first << " -- value: " << std::hex <<
  attrib_it.second << std::dec << std::endl;
  }
  */

  while (true) {
    std::string user_input;
    std::cout << "Word lookup (q to quit): " << std::endl;
    std::getline(std::cin, user_input);

    std::transform(user_input.begin(), user_input.end(), user_input.begin(),
                   ::toupper);

    if (user_input == "Q") {
      break;
    }

    bool word_found = tower_combiner.check_combination(user_input);
    if (word_found) {
      auto combo_props = tower_combiner.make_wordcombination(user_input);
      std::cout << user_input << " properties:\n" << combo_props << std::endl;
    }
  }
}

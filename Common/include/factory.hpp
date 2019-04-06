/* factory.hpp -- part of the DietyTD Common implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_FACTORY_HPP
#define TD_FACTORY_HPP

#include <map>

template <class ProductType, typename KeyType, typename ProductCreator>
class Factory {
public:
  using key_t = KeyType;
  using product_t = ProductType;
  using product_generator_t = ProductCreator;

  ProductType *create_product(const KeyType &id) const {
    auto product_iter = creator_map.find(id);
    // if the product is in the map, call the associated ProductCreator function
    if (product_iter != creator_map.end())
      return (product_iter->second)();
    return nullptr;
  }

  template <typename... product_args>
  ProductType *create_product(const KeyType &id,
                              product_args &&... args) const {
    auto product_iter = creator_map.find(id);
    // if the product is in the map, call the associated ProductCreator function
    if (product_iter != creator_map.end())
      return (product_iter->second)(std::forward<product_args>(args)...);
    return nullptr;
  }

  bool register_product(const KeyType &id, ProductCreator creator) {
    // insert returns an std::pair<iterator, bool> --> we want the bool
    return creator_map.insert(std::pair<KeyType, ProductCreator>(id, creator))
        .second;
  }

  bool unregister_product(const KeyType &id) {
    // check that 1 entry was removed
    return (creator_map.erase(id) == 1);
  }

private:
  std::map<KeyType, ProductCreator> creator_map;
};

#endif

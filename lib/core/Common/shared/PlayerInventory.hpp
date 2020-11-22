/* PlayerInventory.hpp -- part of the DietyTD Common implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_PLAYER_INVENTORY_HPP
#define TD_PLAYER_INVENTORY_HPP

#include <array>
#include <iostream>

// Q: what should the inventory metadata have? --> should have information about
// what the item is that's being held in the inventory. At the moment, this
// would be the characters used to generate words (eventually, I could try to
// have other items as well, but for the forseeable future, it'll be the
// characters)
struct InventoryMetadata {
  InventoryMetadata() {}

  explicit InventoryMetadata(std::string letter) : letter(letter) {}

  friend void swap(InventoryMetadata &meta_lhs, InventoryMetadata meta_rhs) {
    using std::swap;
    swap(meta_lhs.letter, meta_rhs.letter);
  }

  // might start using substrings in the future rather than just letters
  std::string letter;
};

struct PlayerInventory {
public:
  static constexpr int NUM_INVENTORY_ROWS = 5;
  static constexpr int NUM_INVENTORY_COLS = 4;
  static constexpr int NUM_INVENTORY_SLOTS =
      NUM_INVENTORY_ROWS * NUM_INVENTORY_COLS;

  PlayerInventory() {
    for (int idx = 0; idx < NUM_INVENTORY_SLOTS; idx++) {
      inventory_occupied[idx] = false;
    }
  }

  // adds a new item to the next available inventory location.
  // the only case I can think of off hand that it'll return false is if the
  // inventory is already full
  bool add_item(InventoryMetadata data); 

  // Q: in what case would a non-void return be useful? --> if I had non-block
  // sized inventory items, such that swapping two items could result in size
  // mis-matches. I have no plans to do this though
  void swap_item(const int item_aidx, const int item_bidx); 
  std::pair<bool, InventoryMetadata> get_item(const int index) const; 
  void remove_item(const int index); 

  std::array<InventoryMetadata, NUM_INVENTORY_SLOTS> inventory_data;
  std::array<bool, NUM_INVENTORY_SLOTS> inventory_occupied;
};

std::ostream& operator<<(std::ostream& stream, 
                     const InventoryMetadata& inventory); 

std::ostream& operator<<(std::ostream& stream, 
                     const PlayerInventory& inventory); 

#endif

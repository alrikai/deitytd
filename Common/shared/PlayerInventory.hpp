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

// Q: what should the inventory metadata have? --> should have information about
// what the item is that's being held in the inventory. At the moment, this would
// be the characters used to generate words (eventually, I could try to have
// other items as well, but for the forseeable future, it'll be the characters)
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
  bool add_item(InventoryMetadata data) {
    bool item_added = false;
    // update the backing data first...
    for (int idx = 0; idx < NUM_INVENTORY_SLOTS; idx++) {
      if (!inventory_occupied[idx]) {
        inventory_data[idx] = data;
        inventory_occupied[idx] = true;
        item_added = true;
        break;
      }
    }

    //... then update the GUI (if the item was added)
    // TODO: update the GUI --> NOTE: this might end up being done elsewhere in
    // the frontend actually...

    return item_added;
  }

  // Q: in what case would a non-void return be useful? --> if I had non-block
  // sized inventory items, such that swapping two items could result in size
  // mis-matches. I have no plans to do this though
  void swap_item(const int item_aidx, const int item_bidx) {
    using std::swap;
    assert(item_aidx >= 0 && item_aidx < NUM_INVENTORY_SLOTS);
    assert(item_bidx >= 0 && item_bidx < NUM_INVENTORY_SLOTS);

    swap(inventory_data[item_aidx], inventory_data[item_bidx]);
  }

    std::pair<bool, InventoryMetadata> get_item(const int index) const
    {
        assert(index >= 0 && index <= NUM_INVENTORY_SLOTS);
        if(inventory_occupied[index]) {
            return std::make_pair(true, inventory_data[index]);
        } else {
            return std::make_pair(false, InventoryMetadata());
        }
    }

    void remove_item(const int index)
    {
        assert(index >= 0 && index <= NUM_INVENTORY_SLOTS);

        //sanity check that we are removing something that does exist
        assert(inventory_occupied[index]);

        //just doing lazy removal, I guess (I think we can get away with this?)
        inventory_occupied[index] = false;
    }

    std::array<InventoryMetadata, NUM_INVENTORY_SLOTS> inventory_data;
    std::array<bool, NUM_INVENTORY_SLOTS> inventory_occupied;
};

#endif

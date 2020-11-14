/* Player.hpp -- part of the DietyTD Common implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_COMMON_SHARED_PLAYER_HPP
#define TD_COMMON_SHARED_PLAYER_HPP

/* This should have the player information -- i.e. if, hypothetically it were a
 * multiplayer game, then the overall game state would consist of some common
 * game state (i.e. the towers that have been built, the mobs present at any
 * given time, etc), and the individual player states (how much $$, # lives,
 * items in inventory, etc). This should encapsulate that player state
 *
 * Will have to re-write the UI parts to obtain their state information from
 * here -- however, it'll have to be done in a thread-safe manner, as the player
 * information should live primarily in the backend.
 *
 * Maybe I can have it where at each iteration, the frontend gets a snapshot of
 * the current player state, and it uses that player state for that ieration
 * (or, maybe update it every N ms if it's expensive), and I have the backend
 * write to it w/o synchronization (i.e. the backend will read/write w/o
 * synchronization, then at the end of the backend iteration (or every N ms,
 * whichever) it'll make a deep copy of the player state and (with
 * synchronzation) write that state to a shared location (shared w/ the
 * frontend), so that then on the next frotnend iteration it'll read from the
 * shared location (protected by a mutex) and use that value for it's front-end
 * tasks, until it reads the updted value the next time.
 *
 * Also, if there's no change in state... well, we'll have to see how expensive
 * the deep copy is. SHould be pretty cheap though (if the mutex causes
 * performance problems, maybe also have an atomic bool flag for whether to read
 * the value -- backend sets it, frontend reads it (and resets it).
 */

#include "PlayerInventory.hpp"
#include <iostream>

struct TDPlayerInformation {
  TDPlayerInformation(int num_lives, int num_essence, int num_gold)
      : num_lives(num_lives), num_essence(num_essence), num_gold(num_gold) {

    // TODO: just for experimentation, create some letter items and add them to
    // the inventory...
    InventoryMetadata letter_a("a");
    InventoryMetadata letter_c("c");
    InventoryMetadata letter_e("e");

    inventory.add_item(letter_a);
    inventory.add_item(letter_c);
    inventory.add_item(letter_e);
  }

  inline int get_num_lives() const { return num_lives; }

  inline void lose_life() {
    std::cout << "YOU'RE DYING!!!" << std::endl;
    //NOTE: have the #lives decrease until we hit 0
    if (num_lives > 0) {
        num_lives--;
    }

    // TODO: send a signal that it is game over
    if (num_lives == 0) {
        std::cout << "Game Over, we are at 0 lives" << std::endl;
    }
  }

  inline void gain_life() { num_lives++; }

  inline int get_num_gold() const { return num_gold; }
  inline int get_num_essence() const { return num_essence; }
  //NOTE: delta can be > 0 or < 0
  //TODO: rely on the caller to validate if the change is possible?
  inline void update_gold(const int delta) { 
      num_gold += delta; 
      if (num_gold < 0) {
          throw std::runtime_error("ERROR: gold is < 0 (" + std::to_string(num_gold) + ")");
      }
  }
  inline void update_essence(const int delta) { 
      num_essence += delta; 
        if (num_essence < 0) {
          throw std::runtime_error("ERROR: essence is < 0 (" + std::to_string(num_essence) + ")");
      }
  }

  inline bool add_item(InventoryMetadata data) {
    return inventory.add_item(data);
  }

  inline const PlayerInventory *get_inventory_state() const {
    return &inventory;
  }

  PlayerInventory inventory;
  int num_lives;
  int num_essence;
  int num_gold;
};

#endif

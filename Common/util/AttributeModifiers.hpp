/* AttributeModifiers.hpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef ATTRIBUTE_MODIFIERS_HPP
#define ATTRIBUTE_MODIFIERS_HPP

#include "TowerProperties.hpp"
#include <vector>

class Monster;
class Tower;

// TODO: need to implement these
#warning "Tower Attribute Modifiers don't do anything"

struct tower_attribute_modifier {
  static constexpr uint32_t ID = 0xFFFFFF00;

  virtual ~tower_attribute_modifier() {}
  // this is how the modifier will be added to the tower -- i.e. its
  // tower_property object will be passed in, and the attribute modifier will
  // add itself to the appropriate tower modifier lists, etc. as needed
  inline virtual void apply_modifier(tower_properties &) = 0;
  inline virtual void aggregate_modifier(tower_property_modifier &) = 0;
  inline virtual void scale_modifier(float) = 0;
};

// handles the stat enhancement type modifiers
struct stat_attribute_modifier : tower_attribute_modifier {
  static constexpr uint32_t ID = 0xFFFFFF01;
  virtual ~stat_attribute_modifier() {}
  inline virtual void apply_modifier(tower_properties &stats) override {
    (void)stats;
  }
  inline virtual void
  aggregate_modifier(tower_property_modifier &stats_modifier) override {
    (void)stats_modifier;
  }
  inline virtual void scale_modifier(float score) override { (void)score; }
};

// handles the on-hit, on-death events
struct event_attribute_modifier : tower_attribute_modifier {
  static constexpr uint32_t ID = 0xFFFFFF02;
  virtual ~event_attribute_modifier() {}
  inline virtual void apply_modifier(tower_properties &stats) override {
    (void)stats;
  }
  inline virtual void
  aggregate_modifier(tower_property_modifier &stats_modifier) override {
    (void)stats_modifier;
  }
  inline virtual void scale_modifier(float score) override { (void)score; }

  virtual void apply_on_event(Monster *mob, Tower *tower, float time_delta) {
    (void)mob;
    (void)tower;
    (void)time_delta;
  }
};

struct aura_attribute_modifier : tower_attribute_modifier {
  static constexpr uint32_t ID = 0xFFFFFF03;
  virtual ~aura_attribute_modifier() {}
  inline virtual void apply_modifier(tower_properties &stats) override {
    (void)stats;
  }
  inline virtual void
  aggregate_modifier(tower_property_modifier &stats_modifier) override {
    (void)stats_modifier;
  }
  inline virtual void scale_modifier(float score) override { (void)score; }

  // should take a collection of tower objects within the aura range to modify
  virtual void apply_aura(std::vector<Tower *> &tower) { (void)tower; }

  // should take a collection of monster objects within the aura range to modify
  virtual void apply_aura(std::vector<Monster *> &tower) { (void)tower; }
};

#endif

/* TDEventTypes.hpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_BACKEND_EVENTTYPES_HPP
#define TD_BACKEND_EVENTTYPES_HPP

#include "EventQueue.hpp"

namespace UserTowerEvents {

template <typename BackendType> struct tower_event {
  tower_event() : row_(-1.0f), col_(-1.0f) {}
  tower_event(float row, float col) : row_(row), col_(col) {}

  virtual ~tower_event() {}
  virtual void apply(BackendType *td_backend) = 0;
  virtual void print() const {
    std::cout << *this << std::endl;
  }

  // need the indices to find the targetted tower
  float row_;
  float col_;
};
template <typename backendtype>
std::ostream& operator<<(std::ostream& stream, 
                     const tower_event<backendtype>& event) {
    stream << "tower_event -- [" << event.row_ << ", " << event.col_ << "]";
	return stream;
}

template <typename BackendType>
struct build_tower_event : public tower_event<BackendType> {
  build_tower_event() : tier(0) {}

  // NOTE: if we only ever build the base tower type, then we don't need a tower
  // type ID.
  //      but if we ever add specialized basetowers, then we'll need IDs for
  //      them
  build_tower_event(uint32_t tower_ID, int tower_tier, float row, float col)
      : tower_event<BackendType>(row, col), ID(tower_ID), tier(tower_tier) {}
  build_tower_event (const build_tower_event &) = default; 	

  // need this now, as col_ and row_ are dependant names (either that or access
  // them through "this->")
  using tower_event<BackendType>::col_;
  using tower_event<BackendType>::row_;

  void apply(BackendType *td_backend) override {
    td_backend->make_tower(ID, tier, col_, row_);
  }

  uint32_t ID;
  int tier;
};
template <typename backendtype>
std::ostream& operator<<(std::ostream& stream, 
                     const build_tower_event<backendtype>& event) {
    stream << "build_tower_event -- [" << event.row_ << ", " << event.col_ << "]";
	stream << "ID: " << event.ID << " @ tier " << event.tier;
	return stream;
}

template <typename T, typename BackendType>
struct modify_tower_event : public tower_event<BackendType> {
  modify_tower_event() : modifier() {}

  modify_tower_event(T mod_type, float row, float col)
      : tower_event<BackendType>(row, col), modifier(std::move(mod_type)) {}

  modify_tower_event (const modify_tower_event &) = default; 	

  using tower_event<BackendType>::col_;
  using tower_event<BackendType>::row_;

  void apply(BackendType *td_backend) override {
    td_backend->modify_tower(modifier, col_, row_);
  }

  T modifier;
};
template <typename T, typename backendtype>
std::ostream& operator<<(std::ostream& stream, 
                     const modify_tower_event<T, backendtype>& event) {
    stream << "modify_tower_event -- [" << event.row_ << ", " << event.col_ << "]";
	stream << "modifier: " << event.modifier;
	return stream;
}


template <typename BackendType>
struct print_tower_event : public tower_event<BackendType> {
  print_tower_event() {}

  print_tower_event(float row, float col)
      : tower_event<BackendType>(row, col) {}
  print_tower_event (const print_tower_event &) = default; 	

  using tower_event<BackendType>::col_;
  using tower_event<BackendType>::row_;

  void apply(BackendType *td_backend) override {
    td_backend->print_tower(col_, row_);
  }
};
template <typename backendtype>
std::ostream& operator<<(std::ostream& stream, 
                     const print_tower_event<backendtype>& event) {
    stream << "print_tower_event -- [" << event.row_ << ", " << event.col_ << "]";
	return stream;
}

template <typename BackendType>
struct tower_target_event : public tower_event<BackendType> {
  tower_target_event() : target_row(-1.0f), target_col(-1.0f) {}
  tower_target_event(float row, float col, float t_row, float t_col)
      : tower_event<BackendType>(row, col), target_row(t_row),
        target_col(t_col) {}
  tower_target_event (const tower_target_event &) = default; 	

  using tower_event<BackendType>::col_;
  using tower_event<BackendType>::row_;

  void apply(BackendType *td_backend) override {
    td_backend->tower_target(col_, row_, target_row, target_col);
  }

  // the location to target - this may need some work, in case we have say,
  // homing attacks. Then we would want to have a reference to the targetted
  // mob rather than a location
  float target_row;
  float target_col;
};
template <typename backendtype>
std::ostream& operator<<(std::ostream& stream, 
                     const tower_target_event<backendtype>& event) {
    stream << "tower_target_event -- [" << event.row_ << ", " << event.col_ << "]";
	stream << "target: " << event.target_row << " x " << event.target_col;
	return stream;
}



// TODO: decide on, and write the other ones

template <typename EventType, const std::size_t SIZE = 64>
struct EventQueueType {
  // NOTE: boost spsc_queue needs its elements to be copyable and have a default
  // constructor. Hence no unique_ptr elements using QType =
  // boost::lockfree::spsc_queue<std::shared_ptr<EventType>,
  // boost::lockfree::capacity<SIZE>>;

  using QType = EventQueue<EventType>;
};

} // namespace UserTowerEvents

#endif

#ifndef TD_TYPES_HPP
#define TD_TYPES_HPP

#include "Types.hpp"

//gotta get the build system ironed out better..
#include </home/alrik/boost_1_55_0/boost/lockfree/spsc_queue.hpp>


namespace UserTowerEvents 
{

struct build_tower_event
{
    build_tower_event()
        : tier_(0), row_(-1), col_(-1)
    {}
    
    //NOTE: if we only ever build the base tower type, then we don't need a tower type ID.
    //      but if we ever add specialized basetowers, then we'll need IDs for them
    build_tower_event(int tier, float row, float col)
        : tier_(tier), row_(row), col_(col)
    {}

    int tier_;
    float row_;
    float col_;
};

template <typename T>
struct modify_tower_event
{
   modify_tower_event()
       : modifier_(nullptr), row_(-1), col_(-1)
   {}

   modify_tower_event(T* mod_type, float row, float col)
       : modifier_(mod_type), row_(row), col_(col)
   {}

   T* modifier_;
   float row_;
   float col_;
};

//TODO: decide on, and write the other ones


template <typename EventType, const std::size_t SIZE = 64>
struct EventQueueType
{
    using QType = boost::lockfree::spsc_queue<EventType, boost::lockfree::capacity<SIZE>>;
};

}


#endif

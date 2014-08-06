#ifndef TD_TYPES_HPP
#define TD_TYPES_HPP

#include "Types.hpp"

//gotta get the build system ironed out better..
#include </home/alrik/boost_1_55_0/boost/lockfree/spsc_queue.hpp>


namespace UserTowerEvents 
{

template <typename BackendType>
struct tower_event
{
    tower_event()
        : row_(-1), col_(-1)
    {}

    tower_event(float row, float col)
        : row_(row), col_(col)
    {}

    virtual void apply (BackendType* td_backend) = 0;

    //need the indices to find the targetted tower
    float row_;
    float col_;
};

template <typename BackendType>
struct build_tower_event : public tower_event<BackendType>
{
    build_tower_event()
        : tier_(0)
    {}
    
    //NOTE: if we only ever build the base tower type, then we don't need a tower type ID.
    //      but if we ever add specialized basetowers, then we'll need IDs for them
    build_tower_event(int tier, float row, float col)
        : tower_event<BackendType>(row, col), tier_(tier)
    {}

    //need this now, as col_ and row_ are dependant names (either that or access them through "this->")
    using tower_event<BackendType>::col_;
    using tower_event<BackendType>::row_;

    void apply (BackendType* td_backend) override
    {
        td_backend->make_tower(tier_, col_, row_);
    }

    int tier_;
};

template <typename T, typename BackendType>
struct modify_tower_event : public tower_event<BackendType>
{
    modify_tower_event()
       : modifier_(nullptr)
    {}

    modify_tower_event(T* mod_type, float row, float col)
       :  tower_event<BackendType>(row, col), modifier_(mod_type)
    {}

    using tower_event<BackendType>::col_;
    using tower_event<BackendType>::row_;

    void apply (BackendType* td_backend) override
    {
        td_backend->modify_tower(modifier_, col_, row_);
    }

    T* modifier_;
};

template <typename BackendType>
struct print_tower_event : public tower_event<BackendType>
{
    print_tower_event()
    {}

    print_tower_event(float row, float col)
        : tower_event<BackendType>(row, col) 
    {}

    using tower_event<BackendType>::col_;
    using tower_event<BackendType>::row_;

    void apply (BackendType* td_backend) override
    {
        td_backend->print_tower(col_, row_);
    }

};

//TODO: decide on, and write the other ones


template <typename EventType, const std::size_t SIZE = 64>
struct EventQueueType
{
    using QType = boost::lockfree::spsc_queue<EventType, boost::lockfree::capacity<SIZE>>;
};

}


#endif

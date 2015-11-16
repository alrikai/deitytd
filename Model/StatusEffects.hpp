#ifndef TD_STATUC_EFFECTS_HPP
#define TD_STATUC_EFFECTS_HPP

/*
 * Statuses are sort of a general concept of anything that (temporarily) affects
 * an entities' state. This might be say, a tower that reduces mob armor on attack,
 * or an aura from a Mob that increases movement speed, or a stun effect, etc.
 *
 * Baisically, we want the effects to be as general as possible, but with a few key
 * features;
 * 1. they will be temporary, i.e. last for N seconds, although N could be a very long time
 * 2. they will have tiers, which determine the order in which they are applied
 * 3. they will affect the state of either a Tower or Mob (I dont think they'll overlap, i.e. no statuses that apply to both tower AND mob)
 *
 */

//#include "Towers/Tower.hpp"
//#include "Monster.hpp"

#include "util/Elements.hpp"
#include "factory.hpp"
#include "StatusEffectIDs.hpp"


//this structure should have all the information necessary for creating a given status effect
struct StatusParameters
{
  //... the specific statuses' parameters
  float armor_reduce_amount;
  float flat_attack_amount;
};

struct StatusData
{
    StatusData(const int32_t duration, const int32_t precedence)
        : life_ticks(duration), status_tier(precedence)
    {}

    //number of update ticks for this to live
    int32_t life_ticks;
    int32_t status_tier;
};


//status effects that have some finite lifespan (i.e. an effect that's applied on-hit and lasts N-seconds)
class TimedStatusEffect
{
public:
    TimedStatusEffect(const StatusData& status_metadata) 
        : status_metadata (status_metadata)
    {}

    TimedStatusEffect (const int32_t duration, const int32_t precedence) 
        : status_metadata (StatusData (duration, precedence))
    {
        
    }

    virtual ~TimedStatusEffect() 
    {}
    
    virtual void apply(MonsterStats& mob_attributes)
    {}
    virtual void apply(tower_properties& tower_attributes)
    {}
    
    //called on every round during pre-processing; updates the lifecycle and returns 
    //if the status should be active or should be discarded this cycle
    virtual bool update_lifecycle()
    {
      status_metadata.life_ticks -= 1;
      return (status_metadata.life_ticks <= 0);
    }

    virtual int32_t get_precedencetier() const
    {
      return status_metadata.status_tier;
    } 

private:
    StatusData status_metadata;
};

//-------------------------------------------------------------------------------------------------------------------


class ArmorReduceStatus : public TimedStatusEffect
{
public:
    ArmorReduceStatus (const StatusData& status_metadata, const StatusParameters& status_param) 
        : TimedStatusEffect(status_metadata)
    {
      sunder_amt = status_param.armor_reduce_amount;
    }

    //NOTE: effect has no effect on towers
    void apply(MonsterStats& mob_attributes) override
    {
        mob_attributes.armor_amount -= sunder_amt;
    }  

private:
    float sunder_amt;

    //TODO: need to have some other stuff here... Will probably need some (shared between frontend and backend) enum identifying the TYPE
    //of effect, so that the frontend can display the correct animation, tooltop, icon, etc.
};

//-------------------------------------------------------------------------------------------------------------------


//flat +dmg effect
//NOTE: has no effect on monsters
class FlatAttackBoostStatus : public TimedStatusEffect
{
public:
    FlatAttackBoostStatus (const StatusData& status_metadata, const StatusParameters& status_param) 
        : TimedStatusEffect(status_metadata)
    {
      atkboost_amt = status_param.flat_attack_amount;
    }

    void apply(tower_properties& tower_attributes) override
    {
        for (int element_idx = 0; element_idx < tower_properties::NUM_ELEMENTS; ++element_idx) {
            tower_attributes.damage[element_idx].damage_range.low += atkboost_amt;
            tower_attributes.damage[element_idx].damage_range.high += atkboost_amt;
        }
    }  

private:
    float atkboost_amt;
};

//-------------------------------------------------------------------------------------------------------------------

namespace status_effects {
    namespace detail {
        //generalized constructor, of sorts
        template <typename status_t,typename ... status_args>
        TimedStatusEffect* generate_status_object_ (status_args&& ... args)
        {
            return new status_t(std::forward<status_args>(args) ... );
        }

        template <typename status_t>
        TimedStatusEffect* generate_status_object (const StatusData& sdata, const StatusParameters& sparams)
        {
            return new status_t(sdata, sparams);
        }

        struct status_generator
        {
            using status_factoryfcn_t = std::function<TimedStatusEffect* (const StatusData&, const StatusParameters&)>;
            using status_factory_t = Factory<TimedStatusEffect, STATUS_EFFECTS_IDS, status_factoryfcn_t>;
            status_factory_t status_effect_maker;
    
            status_generator()
            {
                status_effect_maker.register_product(STATUS_EFFECTS_IDS::ARMOR_REDUCE, std::bind(generate_status_object<ArmorReduceStatus>, std::placeholders::_1, std::placeholders::_2)); 
                status_effect_maker.register_product(STATUS_EFFECTS_IDS::FLAT_ATTACK_BOOST, std::bind(generate_status_object<FlatAttackBoostStatus>, std::placeholders::_1, std::placeholders::_2));

                //TODO: register the rest of them... 
            }
        };
    }

    inline std::unique_ptr<TimedStatusEffect> generate_status(const STATUS_EFFECTS_IDS status_id, const StatusData& status_metadata, const StatusParameters& status_params)
    {
        static detail::status_generator status_effect_factory;
        return std::unique_ptr<TimedStatusEffect> (status_effect_factory.status_effect_maker.create_product(status_id, status_metadata, status_params));
    }

}

#endif

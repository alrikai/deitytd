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

#include "Towers/Tower.hpp"
#include "Monster.hpp"

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


class StatusEffect
{
public:
    StatusEffect(const StatusData& status_metadata) 
        : status_metadata (status_metadata)
    {}

    StatusEffect (const int32_t duration, const int32_t precedence) 
        : status_metadata (StatusData (duration, precedence))
    {
        
    }

    virtual ~StatusEffect() 
    {}

    virtual void apply(Monster* mob, Tower* tower) = 0;

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

class ArmorReduceStatus : public StatusEffect
{
public:
    ArmorReduceStatus (const StatusData& status_metadata, const StatusParameters& status_param) 
        : StatusEffect(status_metadata)
    {
      sunder_amt = status_param.armor_reduce_amount;
    }

    //TODO: okay, this design is flawed. Was hoping to be able to change (any of) the parameters via the statuses,
    //but forgot that friendship isn't inherited. So I would need to explicitly declare every subclass status
    //as being friends of the Tower / Monster classes. siiiigh
    //
    //NOTE: effect has no effect on towers
    void apply(Monster* mob, Tower* tower) override
    {
        mob->attributes.armor_amount -= sunder_amt;
    }  

private:
    float sunder_amt;

    //TODO: need to have some other stuff here... Will probably need some (shared between frontend and backend) enum identifying the TYPE
    //of effect, so that the frontend can display the correct animation, tooltop, icon, etc.
};

//-------------------------------------------------------------------------------------------------------------------

//flat +dmg effect
class FlatAttackBoostStatus : public StatusEffect
{
public:
    FlatAttackBoostStatus (const StatusData& status_metadata, const StatusParameters& status_param) 
        : StatusEffect(status_metadata)
    {
      atkboost_amt = status_param.flat_attack_amount;
    }

    //NOTE: has no effect on monsters
    void apply(Monster* mob, Tower* tower) override
    {
        tower->attack_attributes.damage.damage_range.low += atkboost_amt;
        tower->attack_attributes.damage.damage_range.highi += atkboost_amt;
    }  

private:
    float atkboost_amt;
};

//-------------------------------------------------------------------------------------------------------------------

namespace status_effects {
    namespace detail {
        //generalized constructor, of sorts
        template <template status_t,typename ... status_args>
        status_t* generate_status_object (status_args&& ... args)
        {
            return new status_t(std::forward<status_args>(args) ... );
        }


        struct status_generator
        {
          using status_factory = Factory<StatusEffect*, STATUS_EFFECTS_IDS, std::function<StatusEffect* (const StatusData&, const StatusParameters&)>>;
          status_factory status_effect_maker;
    
          generator()
          {
            status_effect_maker.register_product(STATUS_EFFECTS_IDS::ARMOR_REDUCE, std::bind(generate_status_object<ArmorReduceStatus>, std::placeholders::_1, std::placeholders::_2)); 
            status_effect_maker.register_product(STATUS_EFFECTS_IDS::FLAT_ATTACK_BOOST, std::bind(generate_status_object<FlatAttackBoostStatus>, std::placeholders::_1, std::placeholders::_2));

            //TODO: register the rest of them... 
          }
        };
    }

    std::unique_ptr<StatusEffect> generate_status(const STATUS_EFFECTS_IDS status_id, const StatusData& status_metadata, const StatusParameters& status_params)
    {
        static detail::status_generator status_effect_factory;
        return std::unique_ptr<StatusEffect> (status_effect_factory.create_product(status_metadata, status_params));
    }

}

#endif

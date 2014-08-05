#ifndef TD_ESSENCE_SYNTHESIS_HPP
#define TD_ESSENCE_SYNTHESIS_HPP

#include "Essences.hpp"
#include "TowerDispatcher.hpp"

#include <utility>
#include <vector>
#include <memory>



/*
 *  this one will be a bit nastier -- given just the base-class pointers,
 *  we want to be able to invoke the appropriate combinations
 */
template <class dispatcher_t, class ... essence_t>
tower_properties combine_essences__(dispatcher_t& dispatch, essence_t&& ... essences)
{
    tower_properties properties;
    properties = dispatch.invoke(std::forward<essence_t>(essences) ...);
    return properties;
}


//the below parts are fine for static polymorphism -- unfortunately, I don't think
//we'll be able to do that, as we'll need dynamic polymorphism. Unfortunately, C++
//doesnt nativly support multiple-dispatch

template <typename Essence>
void apply_essence(tower_properties& properties, Essence&& c_essence)
{
    c_essence->apply_modifier(properties);
}

template <typename Essence, typename ... Essences>
void apply_essence(tower_properties& properties, Essence&& c_essence, Essences&& ... essences)
{
    apply_essence(properties, std::forward<Essences>(essences) ...);
    c_essence->apply_modifier(properties);
}

//have a set of possible essenceword combinations
template <typename ... Essences>
tower_properties combine_static_essences(Essences&& ... essences)
{
    tower_properties properties;
    apply_essence(properties, std::forward<Essences>(essences) ...);
    return properties;
}

//TODO: populate this with other combinations -- aim for > 30 but < 100
namespace combination_rules 
{
    //2 parameter combinations
    tower_properties combine_essences(aphrodite* aph_essence, apollo* apo_essence);
    tower_properties combine_essences(athena* ath_essence, artemis* art_essence);
    

    //3 parameter combinations
    tower_properties combine_essences(aphrodite* aph_essence, apollo* apo_essence, ares* are_essence);
    tower_properties combine_essences(athena* ath_essence, artemis* art_essence, demeter* dem_essence);


    //the catch-all for non-combinations
    template <typename ... essence_types>
    tower_properties combine_essences(essence_types* ... essences)
    {
        return combine_static_essences(std::forward<essence_types>(essences)...);
    }

}


//registers the allowable combinations and handles the combination dispatching
class tower_generator
{
public:
    tower_generator()
    {
        populate_combinations();
    }

    template <typename ... essences>
    tower_properties  combine(essences*... t_essences)
    {
        const int p_size = sizeof...(t_essences);

        //either do the combination or a regular (linear) combination 
        if((p_size >= 2 && p_size <= 6) && std::get<p_size-2>(dispatcher).is_combination(t_essences ...))
        {
            std::cout << "Making essenceword combination" << std::endl;
            return combine_(t_essences ...);
        }
        else
        {
            std::cout << "Not a essenceword combination" << std::endl;
            return combine_static_essences(t_essences ...);            
        }
    }


private:    
    typedef TowerDispatcher<tower_properties, essence, essence> dispatcher_t2;
    typedef TowerDispatcher<tower_properties, essence, essence, essence> dispatcher_t3;
    typedef TowerDispatcher<tower_properties, essence, essence, essence, essence> dispatcher_t4;
    typedef std::tuple<dispatcher_t2, dispatcher_t3, dispatcher_t4> tower_dispatcher;

    tower_properties combine_(essence* t1, essence* t2)
    {
        auto props = combine_essences__(dispatch2, t1, t2);
        std::cout << "essenceword(2): " << props << std::endl;
		return props;
    }

    tower_properties combine_(essence* t1, essence* t2, essence* t3)
    {
        auto props = combine_essences__(dispatch3, t1, t2, t3);
        std::cout << "essenceword(3): " << props << std::endl;
		return props;
    }    
    
    tower_properties combine_(essence* t1, essence* t2, essence* t3, essence* t4)
    {
        auto props = combine_essences__(dispatch4, t1, t2, t3, t4);
        std::cout << "essenceword(4): " << props << std::endl;
		return props;
    }   

    void populate_combinations()
    {
        //the 2-tower combinations
        dispatch2.add<aphrodite, apollo, combination_rules::combine_essences>();
        dispatch2.add<athena, artemis, combination_rules::combine_essences>();
    
        //the 3-tower combinations
        dispatch3.add<aphrodite, apollo, ares, combination_rules::combine_essences>();
        dispatch3.add<athena, artemis, demeter, combination_rules::combine_essences>();
       
        //the 4-tower combinations
        //...

        dispatcher = std::make_tuple(dispatch2, dispatch3, dispatch4);
    }

    tower_dispatcher dispatcher;

    dispatcher_t2 dispatch2;
    dispatcher_t3 dispatch3;
    dispatcher_t4 dispatch4;
};

#endif

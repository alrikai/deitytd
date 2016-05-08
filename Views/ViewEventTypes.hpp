/* ViewEventTypes.hpp -- part of the DietyTD Views subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */



#ifndef TD_VIEW_EVENT_TYPES_HPP
#define TD_VIEW_EVENT_TYPES_HPP

#include "util/EventQueue.hpp"
#include "Model/TowerModel.hpp"
#include "ModelUtils.hpp"

#include <vector>
#include <string>
#include <memory>

/*********************************************************************
 * the (experimental) backend to frontend events
 *********************************************************************/
  
namespace RenderEvents
{
    struct create_tower
    {
        create_tower(const uint32_t ID, std::shared_ptr<TowerModel> model, const std::string& name, std::vector<float>&& map_offsets) 
          : t_ID(ID), t_model(model), t_name(name), t_map_offsets(std::move(map_offsets)), t_world_offsets {0.0f, 0.0f, 0.0f}
        {
            std::vector<float> dim_avgs (3, 0);
            std::for_each(t_model->polygon_points_.begin(), t_model->polygon_points_.end(), [&dim_avgs]
                  (const std::vector<float>& pt)
                  {
                      dim_avgs[0] += pt[0];
                      dim_avgs[1] += pt[1];
                      dim_avgs[2] += pt[2];
                  });
  
            //get the average coordinate in WORLD COORDINATES. Also NOTE: Since the fractals are 
            //generated as [row, col, depth], we need to shuffle the indices to [col, row, depth]
            const size_t fractal_ptfactor = t_model->polygon_points_.size();
            t_world_offsets[0] = dim_avgs[1] / fractal_ptfactor;
            t_world_offsets[1] = dim_avgs[0] / fractal_ptfactor;
            t_world_offsets[2] = dim_avgs[2] / fractal_ptfactor;
        }
  
        const uint32_t t_ID;
        std::shared_ptr<TowerModel> t_model;
        std::string t_name;
        std::vector<float> t_map_offsets;
        std::vector<float> t_world_offsets;
    };


    struct create_attack
    {
        //create_attack(const std::string& atk_name, const std::vector<float>& location, const std::vector<float>& destination)
        create_attack(const std::string& atk_name, const uint32_t origin_id, const std::string& origin_tower, const std::vector<float>& destination)
            : name(atk_name), origin_tid(origin_id), origin_tname(origin_tower), target(destination)
        {}

        const std::string name;
        const uint32_t origin_tid;
        const std::string origin_tname;
        const std::vector<float> target;
    };

    //the question is, how much do we move the attack per update? It should reflect the backend state, but then we'd have to 
    //translate between front and backend coordinates better.... or that could even be PART of the attack, not part of the move
    //update? --> for now, just make something up
    struct move_attack
    {
        move_attack(const std::string& atk_name, const uint32_t origin_id, const std::string& origin_tower, const std::vector<float>& movement, float time_duration)
            : name(atk_name), origin_tid(origin_id), origin_tname(origin_tower), delta(movement), duration(time_duration) 
        {}

        const std::string name;
        const uint32_t origin_tid;
        const std::string origin_tname;
        const std::vector<float> delta;
        const float duration;
    };

    struct remove_attack
    {
        remove_attack(const std::string& atk_name)
            : name(atk_name)
        {}

        const std::string name;
    };
//NOTE: since we also have to animate characters (and possibly anything else), we should probably look at
//ways to do this in a more generic manner. 
//TODO: for now we are going to just have a secondary event queue arrangement for the mobs, but the end
//goal will be to extract the core categories (i.e. creation, removal, movement, etc) and using some 
//combination of polymorphism and genericity to keep the number of events and event queues to a manageable
//number (otherwise we'll have dozens of largely overlapping event types and event queues)
    struct create_mob
    {
        create_mob(const CharacterModels::ModelIDs id, const std::string& name, std::vector<float>&& map_offsets) 
          : model_id(id), m_name(name), m_map_offsets(std::move(map_offsets))
        {}
  
        const CharacterModels::ModelIDs model_id;
        std::string m_name;
        std::vector<float> m_map_offsets;
    };

    struct move_mob
    {
        move_mob(const std::string& mob_name, const std::vector<float>& movement, float time_duration)
            : name(mob_name), delta(movement), duration(time_duration) 
        {}

        const std::string name;
        const std::vector<float> delta;
        const float duration;
    };

    struct remove_mob
    {
        remove_mob(const std::string& mob_name)
            : name(mob_name)
        {}

        const std::string name;
    };


    //request for information of the selected unit (e.g. mob or tower)
    //TODO: how to do this efficiently? 
    struct unit_information
    {
        //information about the selected unit
        std::string unit_name;
        std::vector<float> m_map_offsets;
 
        //this seems rather limiting, as it'll either be 'tower or not'. At the moment I can only
        //envision getting information about mobs and towers, and even then, maybe not mobs?
        bool is_tower;

        //-------------------------------------

        std::string base_stats;
        std::string current_stats;

        std::string information;
        //TODO: how to make the portrait?
    };

    //Q: do we even need the old state?
    struct state_transition
    {
        GAME_STATE old_state;
        GAME_STATE new_state;
    };
} //namespace RenderEvents



/*
 * This class should hold the backend to frontend events -- the gameloop will create this
 * classes object, then register it with the front and backend. Then the backend will write 
 * the events to be sent to the frontend into here, and the frontend will grab and process 
 * events from here during its render cycle
 */
class ViewEvents
{
public:
    using MakeTowerQueueType = EventQueue<RenderEvents::create_tower>; 
    using MakeAttackQueueType = EventQueue<RenderEvents::create_attack>;
    using MoveAttackQueueType = EventQueue<RenderEvents::move_attack>;
    using RemoveAttackQueueType = EventQueue<RenderEvents::remove_attack>;

    using MakeMobQueueType = EventQueue<RenderEvents::create_mob>; 
    using MoveMobQueueType = EventQueue<RenderEvents::move_mob>;
    using RemoveMobQueueType = EventQueue<RenderEvents::remove_mob>;

    using UnitInfoQueueType = EventQueue<RenderEvents::unit_information>;
    using StateTransitionQueueType = EventQueue<RenderEvents::state_transition>;

    enum class EventTypes 
    { 
      MakeTower,              DestroyTower, 
      MakeAttack, MoveAttack, DestroyAttack, 
      MakeMob,    MoveMob,    DestroyMob,
      UnitInfo,
      StateTransition
    };

    ViewEvents()
    {
        maketower_evtqueue = std::unique_ptr<MakeTowerQueueType>(new MakeTowerQueueType());
        makeattack_evtqueue = std::unique_ptr<MakeAttackQueueType>(new MakeAttackQueueType());
        moveattack_evtqueue = std::unique_ptr<MoveAttackQueueType>(new MoveAttackQueueType());
        removeattack_evtqueue = std::unique_ptr<RemoveAttackQueueType>(new RemoveAttackQueueType());
        
        makemob_evtqueue = std::unique_ptr<MakeMobQueueType>(new MakeMobQueueType());
        movemob_evtqueue = std::unique_ptr<MoveMobQueueType>(new MoveMobQueueType());
        removemob_evtqueue = std::unique_ptr<RemoveMobQueueType>(new RemoveMobQueueType());

        unitinfo_evtqueue = std::unique_ptr<UnitInfoQueueType>(new UnitInfoQueueType());
        statetransition_evtqueue = std::unique_ptr<StateTransitionQueueType>(new StateTransitionQueueType());
    }

    /////////////////////////////////////////////////////////////////////
    void add_maketower_event(std::unique_ptr<RenderEvents::create_tower> evt)
    {
        maketower_evtqueue->push(std::move(evt)); 
    }

//---------------------------------------------------------------------------------------------------------

    void add_makeatk_event(std::unique_ptr<RenderEvents::create_attack> evt)
    {
        makeattack_evtqueue->push(std::move(evt));
    }
    void add_moveatk_event(std::unique_ptr<RenderEvents::move_attack> evt)
    {
        moveattack_evtqueue->push(std::move(evt));
    }
    void add_removeatk_event(std::unique_ptr<RenderEvents::remove_attack> evt)
    {
        removeattack_evtqueue->push(std::move(evt));
    }

//---------------------------------------------------------------------------------------------------------

    void add_makemob_event(std::unique_ptr<RenderEvents::create_mob> evt)
    {
        makemob_evtqueue->push(std::move(evt)); 
    }
    void add_movemob_event(std::unique_ptr<RenderEvents::move_mob> evt)
    {
        movemob_evtqueue->push(std::move(evt));
    }
    void add_removemob_event(std::unique_ptr<RenderEvents::remove_mob> evt)
    {
        removemob_evtqueue->push(std::move(evt));
    }


    void add_unitinfo_event(std::unique_ptr<RenderEvents::unit_information> evt)
    {
        unitinfo_evtqueue->push(std::move(evt));
    }

    void add_statetransition_event(std::unique_ptr<RenderEvents::state_transition> evt)
    {
        statetransition_evtqueue->push(std::move(evt));
    }
//---------------------------------------------------------------------------------------------------------

    template <typename ViewFcn>
    void apply_towerbuild_events(ViewFcn& vfcn)
    {
        execute_event_type<MakeTowerQueueType, ViewFcn>(maketower_evtqueue.get(), vfcn);
    }

//---------------------------------------------------------------------------------------------------------

    template <typename ViewFcn>
    void apply_attackbuild_events(ViewFcn& vfcn)
    {
        execute_event_type<MakeAttackQueueType, ViewFcn>(makeattack_evtqueue.get(), vfcn);
    }    

    template <typename ViewFcn>
    void apply_attackmove_events(ViewFcn& vfcn)
    {
        execute_event_type<MoveAttackQueueType, ViewFcn>(moveattack_evtqueue.get(), vfcn);
    }        

    template <typename ViewFcn>
    void apply_attackremove_events(ViewFcn& vfcn)
    {
        execute_event_type<RemoveAttackQueueType, ViewFcn>(removeattack_evtqueue.get(), vfcn);
    }           

//---------------------------------------------------------------------------------------------------------

    template <typename ViewFcn>
    void apply_mobbuild_events(ViewFcn& vfcn)
    {
        execute_event_type<MakeMobQueueType, ViewFcn>(makemob_evtqueue.get(), vfcn);
    }

    template <typename ViewFcn>
    void apply_mobmove_events(ViewFcn& vfcn)
    {
        execute_event_type<MoveMobQueueType, ViewFcn>(movemob_evtqueue.get(), vfcn);
    }        

    template <typename ViewFcn>
    void apply_mobremove_events(ViewFcn& vfcn)
    {
        execute_event_type<RemoveMobQueueType, ViewFcn>(removemob_evtqueue.get(), vfcn);
    }           


    template <typename ViewFcn>
    void apply_unitinfo_events(ViewFcn& vfcn)
    {
        execute_event_type<UnitInfoQueueType, ViewFcn>(unitinfo_evtqueue.get(), vfcn);
    }           

    template <typename ViewFcn>
    void apply_statetransition_events(ViewFcn& vfcn)
    {
        execute_event_type<StateTransitionQueueType, ViewFcn>(statetransition_evtqueue.get(), vfcn);
    }           

//---------------------------------------------------------------------------------------------------------
/*
 * The question is, what do we do with the frontend executing the events?
 * We will have N different events, and the frontend has to do various things
 * for all N of them. The 
 */
private:    

    //NOTE: this doesn't actually have to be in this class, as it's written
    template <typename QueueType, typename ViewFcn>
    void execute_event_type(QueueType* evt_queue, ViewFcn& vfcn)
    {
       while(!evt_queue->empty())
       {
           bool got_evt = false;
           auto render_evt = evt_queue->pop(got_evt);
           if(got_evt && render_evt)
           {
               vfcn(std::move(render_evt));
           }
        } 
    }

    std::unique_ptr<MakeTowerQueueType> maketower_evtqueue;
    std::unique_ptr<MakeAttackQueueType> makeattack_evtqueue;
    std::unique_ptr<MoveAttackQueueType> moveattack_evtqueue;
    std::unique_ptr<RemoveAttackQueueType> removeattack_evtqueue;


    std::unique_ptr<MakeMobQueueType> makemob_evtqueue;
    std::unique_ptr<MoveMobQueueType> movemob_evtqueue;
    std::unique_ptr<RemoveMobQueueType> removemob_evtqueue;


    std::unique_ptr<UnitInfoQueueType> unitinfo_evtqueue;
    std::unique_ptr<StateTransitionQueueType> statetransition_evtqueue;
};

#endif

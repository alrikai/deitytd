#ifndef TD_VIEW_EVENT_TYPES_HPP
#define TD_VIEW_EVENT_TYPES_HPP

#include "util/EventQueue.hpp"
#include "Model/TowerModel.hpp"

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
        create_tower(std::shared_ptr<TowerModel> model, const std::string& name, std::vector<float>&& map_offsets) 
          : t_model(model), t_name(name), t_map_offsets(std::move(map_offsets)), t_world_offsets {0.0f, 0.0f, 0.0f}
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
  
        std::shared_ptr<TowerModel> t_model;
        std::string t_name;
        std::vector<float> t_map_offsets;
        std::vector<float> t_world_offsets;
    };


    struct create_attack
    {
        //create_attack(const std::string& atk_name, const std::vector<float>& location, const std::vector<float>& destination)
        create_attack(const std::string& atk_name, const std::string& origin_tower, const std::vector<float>& destination)
            : name(atk_name), origin_id(origin_tower), target(destination)
        {}

        const std::string name;
        const std::string origin_id;
        const std::vector<float> target;
    };

    //the question is, how much do we move the attack per update? It should reflect the backend state, but then we'd have to 
    //translate between front and backend coordinates better.... or that could even be PART of the attack, not part of the move
    //update? --> for now, just make something up
    struct move_attack
    {
        move_attack(const std::string& atk_name, const std::string& origin_tower, const std::vector<float>& movement)
            : name(atk_name), origin_id(origin_tower), delta(movement) 
        {}

        const std::string name;
        const std::string origin_id;
        const std::vector<float> delta;
    };

    struct remove_attack
    {
        remove_attack(const std::string& atk_name)
            : name(atk_name)
        {}

        const std::string name;
    };
};


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

    enum class EventTypes { MakeTower, DestroyTower, MakeAttack, MoveAttack, DestroyAttack };

    ViewEvents()
    {
        maketower_evtqueue = std::unique_ptr<MakeTowerQueueType>(new MakeTowerQueueType());
        makeattack_evtqueue = std::unique_ptr<MakeAttackQueueType>(new MakeAttackQueueType());
        moveattack_evtqueue = std::unique_ptr<MoveAttackQueueType>(new MoveAttackQueueType());
        removeattack_evtqueue = std::unique_ptr<RemoveAttackQueueType>(new RemoveAttackQueueType());
    }

    /////////////////////////////////////////////////////////////////////
    void add_maketower_event(std::unique_ptr<RenderEvents::create_tower> evt)
    {
        maketower_evtqueue->push(std::move(evt)); 
    }
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


    template <typename ViewFcn>
    void apply_towerbuild_events(ViewFcn& vfcn)
    {
        execute_event_type<MakeTowerQueueType, ViewFcn>(maketower_evtqueue.get(), vfcn);
    }

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
};

#endif

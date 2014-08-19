#ifndef TD_VIEW_EVENT_TYPES_HPP
#define TD_VIEW_EVENT_TYPES_HPP

#include "util/EventQueue.hpp"
#include "Model/TowerModel.hpp"

#include <vector>
#include <string>


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
    using MakeTowerQType = EventQueue<create_tower>;


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
    using MakeAttackQType = EventQueue<create_attack>;

    //the question is, how much do we move the attack per update? It should reflect the backend state, but then we'd have to 
    //translate between front and backend coordinates better.... or that could even be PART of the attack, not part of the move
    //update? --> for now, just make something up
    struct move_attack
	{
        move_attack(const std::string& atk_name, const std::vector<float>& movement)
            : name(atk_name), delta(movement) 
        {}

		const std::string name;
		const std::vector<float> delta;

	};
    using MakeAttackMoveQType = EventQueue<move_attack>;


};


#endif

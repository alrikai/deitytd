#include "TowerLogic.hpp"


bool TowerLogic::make_tower(const int tier, const float x_coord, const float y_coord)
{
    //the tower name and everything would have to be specified by the user when they make it, but until 
    //we implement a GUI we'll have to simulate that part here
    //pick a random tower from the tower model list 
    std::random_device rdev{};
    std::default_random_engine eng{rdev()};
    std::uniform_int_distribution<> dis(0, tower_models.size()-1);
    auto selected_tower = tower_models.begin();
    std::advance(selected_tower, dis(eng));

    std::vector<float> map_offsets {0.0f, 0.0f, 0.0f};
    //using the user-coords, snap the tower model to the appropriate game map tile
    if(map.is_obstructed(x_coord, y_coord))
    {
        std::cout << "Cannot build at [" << x_coord << ", " << y_coord << "] -- is obstructed!" << std::endl; 
        //TODO: write an error message (with position information) to the backend error queue (to be read by the gameloop
        //and displayed by the frontend at the specified location)
        return false; 
    }


    //we can assume that the location is valid now
    //TODO: need to have other checks, i.e. if the player has enough $$ to build it
    //TODO: need to update whatever datastructures need the towers, etc.

    //get the center in map coordinates of the selected location
    auto tile_offset = map.get_tile_center(x_coord, y_coord);
    map_offsets[0] = tile_offset.col;
    map_offsets[1] = tile_offset.row;
    
    //set the tile to be obstructed
    map.set_obstructed(x_coord, y_coord, true);
    auto tilename = map.get_bounding_tile(x_coord, y_coord);
 
	//FOR TESTING: generate a random tower
    //add the tower to a backing list -- decide how this should interact with the GameMap
    const std::string tower_name {"tower_" + selected_tower->first + "__" + std::to_string(tilename.col) + "_" + std::to_string(tilename.row)};
    t_list[tilename.row][tilename.col] = TowerGenerator::make_fundamentaltower(tier, tower_name); 
    std::cout << "Generating Tower: " << *(t_list[tilename.row][tilename.col].get()) << "@ " << tilename.row << ", " << tilename.col << std::endl;   

    
    //get the average for each dimension (to get the center point)
    std::vector<float> dim_avgs (3, 0);
    std::for_each(selected_tower->second.polygon_points_.begin(), selected_tower->second.polygon_points_.end(), [&dim_avgs]
            (const std::vector<float>& pt)  
            {
                dim_avgs[0] += pt[0];           
                dim_avgs[1] += pt[1];           
                dim_avgs[2] += pt[2];       
            });

    //get the average coordinate in WORLD COORDINATES. Also NOTE: Since the fractals are 
    //generated as [row, col, depth], we need to shuffle the indices to [col, row, depth]
    std::vector<float> world_offsets {0.0f, 0.0f, 0.0f};
    const size_t fractal_ptfactor = selected_tower->second.polygon_points_.size();
    world_offsets[0] = dim_avgs[1] / fractal_ptfactor;
    world_offsets[1] = dim_avgs[0] / fractal_ptfactor;
    world_offsets[2] = dim_avgs[2] / fractal_ptfactor;

    //notify the frontend that a tower has been made
    std::unique_ptr<RenderEvents::create_tower> t_evt = std::unique_ptr<RenderEvents::create_tower>
               (new RenderEvents::create_tower(t_list[tilename.row][tilename.col]->get_model(), tower_name, std::move(map_offsets)));
    td_frontend_events->add_maketower_event(std::move(t_evt));
    
    return true;
}


bool TowerLogic::modify_tower(essence* modifier, const float x_coord, const float y_coord)
{
    //check if tower is at the specified position
    if(!map.is_obstructed(x_coord, y_coord))
        return false;

    auto t_tile = map.get_bounding_tile(x_coord, y_coord);
    return t_list[t_tile.row][t_tile.col]->add_modifier(tower_gen, modifier);
}


bool TowerLogic::print_tower(const float x_coord, const float y_coord)
{
    if(!map.is_obstructed(x_coord, y_coord))
        return false;

    auto t_tile = map.get_bounding_tile(x_coord, y_coord);
    //this would eventually be displayed to the GUI rather than printed to the command line...
    std::cout << *(t_list[t_tile.row][t_tile.col]) << std::endl;
    return true;
}

//
//  TODO: the below mehods are incomplete
//


bool TowerLogic::tower_taget(const float tower_xcoord, const float tower_ycoord, const float target_xcoord, const float target_ycoord)
{
    if(!map.is_obstructed(tower_xcoord, tower_ycoord))
        return false;

    auto t_tile = map.get_bounding_tile(tower_xcoord, tower_ycoord);

    //TODO: set the tower's current target -- still need to decide how exactly this should be done (e.g. are we targetting a location or a mob?)
    //t_list[t_tile.row][t_tile.col]->set_target(target_xcoord, target_ycoord);

    return true;
}


//NOTE: we might want to return a list of generated tower attacks from here?
void TowerLogic::cycle_update(const uint64_t onset_timestamp, std::list<std::shared_ptr<TowerAttack>>& new_attacks)
{
    for (int t_row = 0; t_row < GameMap::MAP_HEIGHT; ++t_row)
    {
        for (int t_col = 0; t_col < GameMap::MAP_WIDTH; ++t_col)
        {
            if(t_list[t_row][t_col])
            {
                //TODO: apply updates and trigger attack if ready and mob in range
                //...
                //
                 
            }
        }
    }
}




/*
 * the question is, what kind of things should we handle in the backend thread,
 * and what kind of things do we handle in the gameloop thread? need to get a 
 * clear idea of this asap
 *
void TowerLogic::backend_evtloop()
{
    while(backend_continue.load())
    {
        //handle tower creation events
        UserTowerEvents::build_tower_event t_evt;
        while(!tbuild_queue->empty())  
        {
            tbuild_queue->pop(t_evt); 
            //check here for: 
            //1. does the player have enough $$
            //2. is the selected maptile obstructed
            //???
            //if the above all check out, then make the tower!

            generate_tower(t_evt.tier_);
        }
    }
}

void TowerLogic::start_backend()
{
     backend_continue.store(true);
     backend_thread = std::unique_ptr<std::thread>(new std::thread(&TowerLogic::backend_evtloop, this));
}

void TowerLogic::register_tower_build_queue(std::shared_ptr<UserTowerEvents::BuildTowerEventQueueType> build_queue);
{
    tbuild_queue = build_queue;     
}
*/



//some testing code for generating randomized towers

/*
namespace TowerTesting
{
    template <class... Args>
    struct essence_typelist
    {
       template <std::size_t N>
       using ttype = typename std::tuple_element<N, std::tuple<Args...>>::type;
    };

    template <std::size_t N, typename ... RuneTypes>
    struct generate_essencelist;

    template <typename Rune, typename ... RuneTypes>
    struct generate_essencelist<1, Rune, RuneTypes ...>
    {
        static void generate(std::vector<std::unique_ptr<essence>>& essence_list)
        {
     	    //using rtype = typename essence_typelist<Rune>::ttype<0>;
            essence_list[0] = std::unique_ptr<essence>(new Rune);
        }
    };

    template <std::size_t N, typename Rune, typename ... RuneTypes>
    struct generate_essencelist<N, Rune, RuneTypes ...>
    {
        static void generate(std::vector<std::unique_ptr<essence>>& essence_list)
        {
     	    //using rtype = typename essence_typelist<Rune>::ttype<N-1>;
            essence_list[N-1] = std::unique_ptr<essence>(new Rune);
            generate_essencelist<N-1, RuneTypes ...>::generate(essence_list);
        }
    };

	//for testing: generate a random tower
	std::unique_ptr<Tower> generate_random_tower(const int tier)
	{
		tower_generator tower_gen;

		std::random_device rdev{};
		std::default_random_engine eng{rdev()};
		
        //cap the tower tiers to the currently-valid range
        const int num_essences = std::min(std::max(2,tier+1), 4);

		//need to choose a random set of essences now too
		using essence_types = std::tuple<aphrodite, apollo, ares, artemis, 
						  athena, demeter, dionysus, hades, hephaestus, 
						  hera, hermes, hestia, poseidon, zeus>;
		
		std::vector<std::unique_ptr<essence>> essence_list (std::tuple_size<essence_types>::value);
		//make_essencelist<essence_types, std::tuple_size<essence_types>::value-1>(essence_list);
		//generate_essencelist<std::tuple_size<essence_types>::value, essence_types>::generate(essence_list);
        generate_essencelist<std::tuple_size<essence_types>::value, aphrodite, apollo, ares, artemis,athena, demeter, 
			 dionysus, hades, hephaestus, hera, hermes, hestia, poseidon, zeus>::generate(essence_list);    
						  
						  
		std::uniform_int_distribution<> essence_pick_dist(0, std::tuple_size<essence_types>::value-1);
		tower_properties props;
		switch(num_essences)
		{
		  case 2:
			  props = tower_gen.combine(essence_list.at(essence_pick_dist(eng)).get(), essence_list.at(essence_pick_dist(eng)).get());
			  break;
		  case 3:
			  props = tower_gen.combine(essence_list.at(essence_pick_dist(eng)).get(), essence_list.at(essence_pick_dist(eng)).get(), 
										essence_list.at(essence_pick_dist(eng)).get());
			  break;
		  case 4:
			  props = tower_gen.combine(essence_list.at(essence_pick_dist(eng)).get(), essence_list.at(essence_pick_dist(eng)).get(),
										essence_list.at(essence_pick_dist(eng)).get(), essence_list.at(essence_pick_dist(eng)).get());
			  break;
		  default:
			  std::cout << "Well, this shouldn't have happened..." << std::endl;
		}

		//just have #essences == tier (for now))
		return std::unique_ptr<Tower>(new Tower(std::move(props), num_essences));
	}  

}
*/



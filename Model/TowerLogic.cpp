#include "TowerLogic.hpp"

#include <queue>

template <typename T>
inline T L2dist(T r_diff, T c_diff)
{
    return std::sqrt(r_diff * r_diff + c_diff * c_diff);
}

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

    //get the tile that the (x, y) point falls within
    auto tower_block = map.get_tower_block(x_coord, y_coord);
    if(map.is_obstructed(tower_block))
    {
        std::cout << "Cannot build at [" << x_coord << ", " << y_coord << "] -- is obstructed!" << std::endl; 
        //TODO: write an error message (with position information) to the backend error queue (to be read by the gameloop
        //and displayed by the frontend at the specified location)
        return false; 
    }
  
    //we can assume that the location is valid now
    //TODO: need to have other checks, i.e. if the player has enough $$ to build it
    //TODO: need to update whatever datastructures need the towers, etc.

    //get the center in map coordinates of the selected location, taking the tower size into accound
    auto block_offset = map.get_block_center(tower_block);
    map_offsets[0] = block_offset.col;
    map_offsets[1] = block_offset.row;
    
    //set the tile to be obstructed
    map.set_obstructed(tower_block, true);

    const int tower_row = std::get<0>(tower_block.row) / GameMap::TowerTileHeight;
    const int tower_col = std::get<0>(tower_block.col) / GameMap::TowerTileWidth;
    //add the tower to a backing list -- decide how this should interact with the GameMap
    const std::string tower_name {"tower_" + selected_tower->first + "_" + std::to_string(tower_col) + "_" + std::to_string(tower_row)};

    //TODO: need to refactor t_list to work on TOWER-indices (e.g. [MAP_HEIGHT/TowerTileHeight][MAP_WIDTH/TowerTileWidth] array)
   
    t_list[tower_row][tower_col] = TowerGenerator::make_fundamentaltower(tier, tower_name, block_offset.row, block_offset.col); 
    std::cout << "Generating Tower: " << "@ [" << block_offset.row << ", " << block_offset.col << "]: \n" << *(t_list[tower_row][tower_col].get()) << std::endl; 
    
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
               (new RenderEvents::create_tower(t_list[tower_row][tower_col]->get_model(), tower_name, std::move(map_offsets)));
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
    
    //TODO: set the tower's current target -- still need to decide how exactly this should be done (e.g. are we targetting a location or a mob?)
    //t_list[t_tile.row][t_tile.col]->set_target(target_xcoord, target_ycoord);

    return true;
}

//not sure where this function should actually live -- part of TowerLogic, GameMap, or Tower?
bool TowerLogic::get_targets(Tower* tower, const int t_col, const int t_row)
{   
    //get the tower tile extents, in terms of map tiles. location is [inclusive, exclusive)
    int tile_row = t_row * GameMap::TowerTileHeight;
    int tile_col = t_col * GameMap::TowerTileWidth;
    //NOTE: centers of tiles have to be normalized
    /*
    constexpr double THeight = 1.0 / GameMap::MAP_HEIGHT;
    constexpr double TWidth = 1.0 / GameMap::MAP_WIDTH;
    auto tile_center = Coordinate<float>(THeight * (tile_row + GameMap::TowerTileHeight/2.0f), TWidth * (tile_col + GameMap::TowerTileWidth/2.0f));
    */
    auto tile_center = tower->get_position();

    //check if the cached target is still valid
    auto prev_target = tower->get_target();
    if(prev_target != nullptr)
    {
        float mob_row, mob_col;
        std::tie(mob_row, mob_col) = prev_target->get_position();
        auto t_dist = L2dist(mob_row - tile_center.row, mob_col - tile_center.col);            
        //if old target is still in range, nothing else to do
        if(tower->in_range(t_dist))   
            return true;
    }

    using cmp_type = MapTile*;
    //takes the L2 distance of the tiles
    auto L2cmp = [tile_center] (const cmp_type& lhs, const cmp_type& rhs)
    {
        float lhs_dist = L2dist<float>(lhs->tile_center.row - tile_center.row, lhs->tile_center.col - tile_center.col);
        float rhs_dist = L2dist<float>(rhs->tile_center.row - tile_center.row, rhs->tile_center.col - tile_center.col);
        return lhs_dist < rhs_dist;
    };

    std::priority_queue<cmp_type, std::vector<cmp_type>, decltype(L2cmp)> bfs_tiles (L2cmp);

    //add the the tower block tiles to the visited list (can't have a mob in the tower... hopefully...)
    std::map<std::pair<int, int>, bool> visited_tiles;
    for (int row = tile_row; row < tile_row + GameMap::TowerTileHeight; ++row)
        for (int col = tile_col; col < tile_col + GameMap::TowerTileWidth; ++col)
            visited_tiles.emplace(std::make_pair(row, col), true);

    //add the outer perimeter of the tower blocks to the queue
    int target_idx_low = tile_row - 1;
    int target_idx_high = tile_row + GameMap::TowerTileHeight;
    for (int col = tile_col-1; col < tile_col + GameMap::TowerTileWidth+1; ++col)
    {
        if(col >= 0 && col < GameMap::MAP_WIDTH)
        {
            if(target_idx_low >= 0)
            {
                bfs_tiles.push(map.get_tile(col, target_idx_low));
                visited_tiles.emplace(std::make_pair(target_idx_low, col), true);
            }

            if(target_idx_high < GameMap::MAP_HEIGHT)
            {
                bfs_tiles.push(map.get_tile(col, target_idx_high));
                visited_tiles.emplace(std::make_pair(target_idx_high, col), true);
            }
        }
    }

    target_idx_low = tile_col - 1;
    target_idx_high = tile_col + GameMap::TowerTileWidth;
    //reduce iteration count by 1 on both sides to avoid duplicates
    for (int row = tile_row; row < tile_row + GameMap::TowerTileHeight; ++row)
    {
        if(row >= 0 && row < GameMap::MAP_HEIGHT)
        {
            if(target_idx_low >= 0)
            {
                bfs_tiles.push(map.get_tile(target_idx_low, row));
                visited_tiles.emplace(std::make_pair(row, target_idx_low), true);
            }

            if(target_idx_high < GameMap::MAP_WIDTH)
            {
                bfs_tiles.push(map.get_tile(target_idx_high, row));
                visited_tiles.emplace(std::make_pair(row, target_idx_high), true);
            }
        }
    }

    while(!bfs_tiles.empty())  
    {
        auto target_tile = bfs_tiles.top();
        bfs_tiles.pop();       
         
        //found a tile with mobs. Need to select the closest one to target
        if(!target_tile->resident_mobs.empty())
        {
            auto t_dist = L2dist(target_tile->tile_center.row - tile_center.row, 
                                 target_tile->tile_center.col - tile_center.col);            
            std::cout << "Suitable tile @ [" << target_tile->idx_location.row << ", " << target_tile->idx_location.col << "] -- has " 
                      << target_tile->resident_mobs.size() << " #resident mobs @ " << t_dist << " units away" << std::endl; 

            //TODO: for now we just take an arbitrary mob within the tile
            auto target_mob = target_tile->resident_mobs.front();
            tower->set_target(target_mob.get());
            return true;
        }

        //add the 8 neighbors of the current tile (if in range)
        for (int n_row = target_tile->idx_location.row - 1; n_row <= target_tile->idx_location.row + 1; ++n_row)
        {
            for (int n_col = target_tile->idx_location.col - 1; n_col <= target_tile->idx_location.col + 1; ++n_col)
            {
                //boundary checks -- can't exceed map dimensions
                if(n_row < 0 || n_row >= GameMap::MAP_HEIGHT || n_col < 0 || n_col >= GameMap::MAP_WIDTH)   
                    continue;
              
                //dont examine tiles that have already been seen
                auto tile_it = visited_tiles.find(std::make_pair(n_row, n_col));
                if(tile_it != visited_tiles.end())
                    continue;

                //boundary checks -- see if the targets are within range.
                //TODO: decide on a better/more precise distance measure. We are just comparing the middle of the tiles, which isn't as
                //accurate as it could be. i.e. we could compare from the edge of the tower, or compare to the actual mobs IN the tile
                //(i.e. we'll have mobs resident on the tile, but they might be further than the tile mid-point and hence out of range)
                auto t_dist = L2dist(map.get_tile(n_col, n_row)->tile_center.row - tile_center.row, 
                                     map.get_tile(n_col, n_row)->tile_center.col - tile_center.col);
                if(!tower->in_range(t_dist))    
                    continue;

                bfs_tiles.push(map.get_tile(n_col, n_row));
                visited_tiles.emplace(std::make_pair(n_row, n_col), true);
            } 
        }
    }

    //nothing was in range, tower has no target
    tower->set_target(nullptr);
    return false;
}

//NOTE: we might want to return a list of generated tower attacks from here?
void TowerLogic::cycle_update(const uint64_t onset_timestamp)
{
    //cycle through the attacks and remove the finished ones
    auto attack_it = active_attacks.begin();
    while(attack_it != active_attacks.end())
    {
        //get rid of attacks that are out of bounds (e.g. if they missed)
        if (!(*attack_it)->in_bounds())
        {
            //signal the frontend to remove the attack
            std::unique_ptr<RenderEvents::remove_attack> t_evt = std::unique_ptr<RenderEvents::remove_attack>
                        (new RenderEvents::remove_attack((*attack_it)->get_id()));
            td_frontend_events->add_removeatk_event(std::move(t_evt));     
            
            //remove the attack internally
            attack_it = active_attacks.erase(attack_it);
            continue;
        }

        //what other things to check? --> collisions, timers (e.g. if the attack explodes after N seconds), etc.
        if((*attack_it)->hit_target())
        {
            //std::cout << "Attack " << (*attack_it)->get_id() << " hit target!" << std::endl;

            //we would trigger the attack on-hit animation here...
            //... but instead, signal the frontend to remove the attack
            std::unique_ptr<RenderEvents::remove_attack> t_evt = std::unique_ptr<RenderEvents::remove_attack>
                        (new RenderEvents::remove_attack((*attack_it)->get_id()));
            td_frontend_events->add_removeatk_event(std::move(t_evt));     
            
            //remove the attack internally
            attack_it = active_attacks.erase(attack_it);
            continue;        
        }

        attack_it++;
    }

    //perform the tower updates
    for (int t_row = 0; t_row < TLIST_HEIGHT; ++t_row)
    {
        for (int t_col = 0; t_col < TLIST_WIDTH; ++t_col)
        {
            if(t_list[t_row][t_col] != nullptr)
            {
                //TODO: apply updates and trigger attack if ready and mob in range
                //...
                //
   
                bool debug_trigger = onset_timestamp % 30 == 0;

                //trigger attack if mob in range (ignoring attack speed, user-specified targetting, and prior targets 
                bool mob_in_range = get_targets(t_list[t_row][t_col].get(), t_col, t_row);
                if(mob_in_range && debug_trigger)
                {
                    //spawn attack -- will need to take attack speed into account (maybe prior to checking the range?)
                    
                    const std::string origin_tower_id = t_list[t_row][t_col]->get_id();
                    const std::string attack_id = origin_tower_id + "_attack_" + std::to_string(onset_timestamp);

                    //get the normalized position of the target -- convert to tile position 
                    auto attack_target = t_list[t_row][t_col]->get_target();
                    float mob_row, mob_col;
                    std::tie(mob_row, mob_col) = attack_target->get_position();
                    const int mob_tile_row = std::floor(mob_row/GameMap::NormFactorHeight);
                    const int mob_tile_col = std::floor(mob_col/GameMap::NormFactorWidth);
                    std::vector<float> target {static_cast<float>(mob_tile_col), static_cast<float>(mob_tile_row), 0.0f};

                    //make the attack generation event
                    std::unique_ptr<RenderEvents::create_attack> t_evt = std::unique_ptr<RenderEvents::create_attack>
                        (new RenderEvents::create_attack(attack_id, origin_tower_id, std::move(target)));
                    td_frontend_events->add_makeatk_event(std::move(t_evt));

                    //what parameters to have? perhaps a name and a timestamp?
                    auto t_attack = t_list[t_row][t_col]->generate_attack(attack_id, onset_timestamp);
                    t_attack->set_target(Coordinate<float>(mob_col, mob_row));
                    active_attacks.emplace_back(std::move(t_attack));
                }
            }
        }
    }

    //update the attack positions, spawn relevant events for the frontend
    for (auto attack_it = active_attacks.begin(); attack_it != active_attacks.end(); ++attack_it)
    {
        if(onset_timestamp % 5 == 0)
        {
        //get the amount the attack should move. Will probably need some time-element   
        auto atk_movement = (*attack_it)->move_update(onset_timestamp);
        const std::vector<float> movement {atk_movement.col, atk_movement.row, 0.0f};

        //use this as a placeholder
        //const std::vector<float> movement {GameMap::TowerTileWidth, GameMap::TowerTileHeight, 0.0f};
        
        auto attack_id = (*attack_it)->get_id();
        auto origin_id = (*attack_it)->get_origin_id();

        auto t_evt = std::unique_ptr<RenderEvents::move_attack>(new RenderEvents::move_attack(attack_id, origin_id, movement, 150.0f));
        td_frontend_events->add_moveatk_event(std::move(t_evt));
        }
    }
}

/*
 Tower Targetting:

 1. is there a user-specified target?
    1a. Is it in range?
        Y: spawn attack
        N: auto-target logic
 Auto Target:
 1. Does the Tower have a target?
    Y: Is it in range?
        Y: spawn attack
        N: retarget
    N: retarget

 Retarget:

*/

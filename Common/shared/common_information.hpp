#ifndef TD_COMMON_SHARED_COMMON_INFORMATION_HPP
#define TD_COMMON_SHARED_COMMON_INFORMATION_HPP

#include <unordered_map>
#include <mutex>
#include <string>
#include <iostream>
#include <sstream>

#include "util/TowerProperties.hpp"
#include "tower_combiner/TowerCombiner.hpp"

/*
 * the idea is to have something here that'll be written to by the backend and read by the frontend,
 * which will be threadsafe and will allow for on-demand information to be obtained by the frontend
 * (rather than going through the usual asynch-queue communication approach). 
 */

//this should have all the information regarding a tower's attributes that the 
//frontend would require
struct CommonTowerInformation
{
    tower_properties base_tower_props;
    int tier;
    int num_wordslots;

	std::string tower_name;
    //TODO: how best to store the tower thumbnail image?
	//
	
	std::string get_tower_info_string() const 
	{
        std::stringstream tinfo_oss;
        tinfo_oss << tower_name << "\nTier: " << tier << " Attributes:\n" << base_tower_props << "\n";
		return tinfo_oss.str();
	}
};



template <typename tower_information_t, typename player_information_t>
class GameInformation
{
public:
    using tower_key_t = uint32_t;
    using tower_info_t = tower_information_t;

    GameInformation(player_information_t player_info)
		: player_info (player_info)
	{}

    void add_new_towerinfo(tower_key_t tower_id, tower_info_t info)
    {
        std::lock_guard<std::mutex> lock (tower_info_mutx);

        auto t_ins = tower_info.insert(std::make_pair(tower_id, info));
        //check if insertion was successful
        if(!t_ins.second) {
            //NOTE: should be impossible, as towers all recieve unique IDs
            std::cout << "ERROR -- insertion of ID " << tower_id << " was unsuccessful" << std::endl;
            std::string error_str {"ERROR -- ID " + std::to_string(tower_id) + " exists already"};
            throw std::logic_error(error_str);
        }

		std::cout << "added " << tower_id << " to gameinfo structure" << std::endl;
    }

    void update_towerinfo(tower_key_t tower_id, tower_info_t info)
    {
        std::lock_guard<std::mutex> lock (tower_info_mutx);

        auto tower_info_it = tower_info.find(tower_id);
        if(tower_info_it != tower_info.end()) {
            tower_info_it->second = info;
        } else {
            //NOTE: should be impossible, as towers are added at creation
            std::string error_str {"ERROR -- ID " + std::to_string(tower_id) + " doesnt exist"};
            throw std::logic_error(error_str);
        }
    }

    tower_info_t get_towerinfo(tower_key_t tower_id)
    {
        std::lock_guard<std::mutex> lock (tower_info_mutx);

        auto tower_info_it = tower_info.find(tower_id);
        if(tower_info_it != tower_info.end()) {
            return tower_info_it->second;
        } else {
            //NOTE: should be impossible, as towers are added at creation
            std::string error_str {"ERROR -- ID " + std::to_string(tower_id) + " doesnt exist"};
            throw std::logic_error(error_str);
        }
    }

	//-------------------------------------------------------------------------------
	
	inline void set_player_state_snapshot(player_information_t&& player_state)
	{
        std::lock_guard<std::mutex> lock (player_info_mutx);
		player_info = std::move(player_state);
	}

	inline player_information_t get_player_state_snapshot() const
	{
        std::lock_guard<std::mutex> lock (player_info_mutx);
		return player_info;
	}

private:
    mutable std::mutex tower_info_mutx;
    std::unordered_map<tower_key_t, tower_info_t> tower_info;

    mutable std::mutex player_info_mutx;
    player_information_t player_info;
};


//this is the singleton for the towercombiner. I am still not entirely sure that this is the best way
//to go, but I think I can have it s.t. this is really ONLY used from the frontend (and we just pass
//the tower_properties w/ a modify event to the backend from the frontend... but we'll have to see?)
inline const TowerCombiner& get_towercombiner()
{
	//choose the word dictionary and default modifier stats 
	const static std::string config_file {"resources/default_attribute_values.yaml"};
	const static std::string dict_file {"resources/word_list.txt"};

	static TowerCombiner tower_gen(dict_file, config_file);
	return tower_gen;
}

#endif

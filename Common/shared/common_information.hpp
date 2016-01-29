#ifndef TD_COMMON_SHARED_COMMON_INFORMATION_HPP
#define TD_COMMON_SHARED_COMMON_INFORMATION_HPP

#include <unordered_map>
#include <mutex>
#include <iostream>

#include "util/TowerProperties.hpp"

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
};

template <typename tower_information_t>
class GameInformation
{
public:
    using tower_key_t = uint32_t;
    using tower_info_t = tower_information_t;

    void add_new_towerinfo(tower_key_t tower_id, tower_info_t info)
    {
        std::lock_guard<std::mutex> lock (info_mutx);

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
        std::lock_guard<std::mutex> lock (info_mutx);

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
        std::lock_guard<std::mutex> lock (info_mutx);

        auto tower_info_it = tower_info.find(tower_id);
        if(tower_info_it != tower_info.end()) {
            return tower_info_it->second;
        } else {
            //NOTE: should be impossible, as towers are added at creation
            std::string error_str {"ERROR -- ID " + std::to_string(tower_id) + " doesnt exist"};
            throw std::logic_error(error_str);
        }
    }
private:
    mutable std::mutex info_mutx;
    std::unordered_map<tower_key_t, tower_info_t> tower_info;
};

#endif

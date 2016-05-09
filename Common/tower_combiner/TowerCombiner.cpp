/* TowerCombiner.cpp -- part of the DietyTD Model subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "TowerCombiner.hpp"
#include "ModifierConfigParser.hpp"

#include <fstream>
#include <chrono>

//NOTE: these are largely temporary at the moment... also if I DONT have the +0, I somehow get linker errors on the IDs. Seems 
//silly, but it is what it is... happens on gcc 4.8 and clang 3.5
const std::unordered_map<char, uint32_t> TowerCombiner::character_attribute_map = {
        {'I', flat_damage::ID+0}, {'N', flat_damage::ID+0}, {'O', flat_damage::ID+0}, {'T', flat_damage::ID+0}, {'U', flat_damage::ID+0}, 
        {'E', enhanced_damage::ID+0}, {'D', enhanced_damage::ID+0}, {'Q', enhanced_damage::ID+0}, 
        {'S', enhanced_speed::ID+0}, {'K', enhanced_speed::ID+0},  
        {'R', flat_range::ID+0}, {'L', flat_range::ID+0}, 
        {'C', flat_crit_chance::ID+0}, {'Z', flat_crit_chance::ID+0},
        {'B', flat_crit_multiplier::ID+0},  {'M', flat_crit_multiplier::ID+0}, 
        {'H', flat_type_damage::ID + flat_type_damage::CHAOS_ID},
        {'W', flat_type_damage::ID + flat_type_damage::WATER_ID}, 
        {'A', flat_type_damage::ID + flat_type_damage::AIR_ID},  
        {'F', flat_type_damage::ID + flat_type_damage::FIRE_ID},
        {'G', flat_type_damage::ID + flat_type_damage::EARTH_ID},
        {'X', enhanced_type_damage::ID + enhanced_type_damage::CHAOS_ID},
        {'V', enhanced_type_damage::ID + enhanced_type_damage::WATER_ID}, 
        {'Y', enhanced_type_damage::ID + enhanced_type_damage::AIR_ID},
        {'J', enhanced_type_damage::ID + enhanced_type_damage::FIRE_ID},  
        {'P', enhanced_type_damage::ID + enhanced_type_damage::EARTH_ID}
};

//---------------------------------------------------------------------------------------------------------

namespace {

template <typename dict_t>
void load_dictionary(dict_t& dict, const std::string& dictionary_file)
{
    std::ifstream dict_file(dictionary_file);    
    std::string dict_line;
    while(std::getline(dict_file, dict_line)) {
        dict.emplace(std::make_pair(dict_line, nullptr));    
    }    

    std::cout << "loaded word dict " << dict.size() << std::endl;
}

//computes a word's score based on the scrabble score
uint32_t compute_wordscore(const std::string& word)
{
    static std::unordered_map<char, uint32_t> word_cost_weights = {
        {'A', 1},  {'E', 1}, {'I', 1}, {'L', 1}, {'N', 1}, {'O', 1}, {'R', 1}, {'S', 1}, {'T', 1}, {'U', 1}, 
        {'D', 2},  {'G', 2},
        {'B', 3},  {'C', 3}, {'M', 3}, {'P', 3},
        {'F', 4},  {'H', 4}, {'V', 4}, {'W', 4}, {'Y', 4},
        {'K', 5},
        {'J', 8},  {'X', 8},
        {'Q', 10}, {'Z', 10}
    };

    uint32_t score = 0;
    for (auto c : word) {
        score += word_cost_weights[c]; 
    }
    return score;
}
}
//---------------------------------------------------------------------------------------------------------

TowerCombiner::TowerCombiner(const std::string& dictionary_fpath, const std::string& attribute_cfgfpath)
    : dictionary_filename(dictionary_fpath), attributecfg_filename(attribute_cfgfpath)
{
    std::cout << "loading dictionary at " << dictionary_filename << " -- attribute cfg at " << attributecfg_filename << std::endl;
    //load the dictionary, prepare the data structures
    load_dictionary(dict, dictionary_filename);
    //populaates the modifier factory 
    get_modifier_configs(attribute_fact, attributecfg_filename);
}

//returns the aggregate modifier tower_properties object that results from the given combination
tower_properties TowerCombiner::make_wordcombination(const std::string& word) const
{
    tower_properties props;

    //NOTE: the caller really should check this beforehand, but just have this here for insurance
    if(!check_combination(word)) {
        std::cout << "ERROR -- word " << word << " not a valid word combo" << std::endl;
        return props;
    }

    auto word_score = compute_wordscore (word);

    tower_property_modifier stats_modifier;
    //next, need the list of attributes for the word
    for (auto word_unit : word) {
        auto modifier_key_it = character_attribute_map.find(word_unit);
        if(modifier_key_it != character_attribute_map.end()) {
            std::cout << "character " << word_unit << " --> " << std::hex << modifier_key_it->second << std::dec << std::endl;
            auto attribmodifier = attribute_fact.create_product(modifier_key_it->second, word_score);
            std::cout << typeid(*attribmodifier).name() << std::endl;

            //aggregate the modifier values so we can apply them in a well-ordered manner
            attribmodifier->aggregate_modifier(stats_modifier);
        }
    }

    props.apply_property_modifier(std::move(stats_modifier));
    std::cout << "word " << word << " -- properties: " << props << std::endl;

    return props;
}

bool TowerCombiner::check_combination(const std::string& word) const
{
    auto start_timestamp = std::chrono::high_resolution_clock::now();

    auto dict_it = dict.find(word);
    bool word_lookup_found = dict_it != dict.end();
    //uint32_t word_score = (word_lookup_found ? compute_wordscore(dict_it->first) : 0);

    //just for testing the lookup performance...
    auto end_timestamp = std::chrono::high_resolution_clock::now();
    auto lookup_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_timestamp - start_timestamp).count();
    std::cout << "dict lookup: " << lookup_duration << " us" << std::endl;
    if(word_lookup_found) {
        std::cout << "found " << dict_it->first << std::endl;
    } else {
        std::cout << "couldn't find " << word << std::endl;
    }

    return word_lookup_found;
}

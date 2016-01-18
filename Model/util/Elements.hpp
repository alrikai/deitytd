#ifndef TD_ELEMENTS_HPP
#define TD_ELEMENTS_HPP

/*
 *  The basic plan:
 *
 *  Will have different tiers of tower; i.e. [1 ~ 10).
 *  The user will upgrade their distributions, s.t. they'll have weights for the random selections, 
 *  i.e. it'll center the random distributions around { T1: 20, T2: 40, T3: 60, T4: 80 }
 *
 *  These randomized values will then be within the range [0, 100]. This value will be used in
 *  conjunction with the element distributions for the tower to determine the tower's base stats.
 *
 *  In addition, this randomized value will determine the tower's tier; it'll be binned into the 
 *  tiers [1 ~ 10), which will be combined with the element distributions to determine the tower's
 *  eligibility with the ability pools. 
 *
 *  Will have the user-supplied element distributions, from [0 ~ 100]. The higher the coefficient,
 *  the higher the resultant tower cost.
 *  Will also need to make a tiered pool of abilities. Will have some requirements, e.g. the tower
 *  has to be of a certain tier and have a high enough set of element distributions, then it'll be
 *  able to select that attribute. Let the user select which ones to take. 
 */

#include <memory>
#include <iostream>
#include <map>
//#include "RandomUtility.hpp"

enum class Elements {CHAOS = 0, WATER, AIR, FIRE, EARTH};

//can have the element affinity lookup tables here
namespace ElementInfo
{
    inline std::string get_element_name(const Elements& type)
    {
        const static std::map<Elements, std::string> element_names 
            {{Elements::CHAOS, "Chaos"}, {Elements::WATER, "Water"}, 
            {Elements::AIR, "Air"}, {Elements::FIRE, "Fire"}, {Elements::EARTH, "Earth"}};

        auto elem_it = element_names.find(type);
        if(elem_it != element_names.end())
            return elem_it->second;
        else
            return "";
    }

    //holds the damage coefficients for the elements -- key types are: <attacker, defender>
    //will have to tweak these as time goes on. Also seriously consider putting these into
    //a config file...
    const static std::map<std::tuple<Elements, Elements>, double> damage_coeffs
    {
     {std::make_tuple(Elements::CHAOS, Elements::CHAOS), 1.0},
     {std::make_tuple(Elements::CHAOS, Elements::WATER), 1.0},
     {std::make_tuple(Elements::CHAOS, Elements::AIR),   1.0},
     {std::make_tuple(Elements::CHAOS, Elements::FIRE),  1.0},
     {std::make_tuple(Elements::CHAOS, Elements::EARTH), 1.0},

     {std::make_tuple(Elements::WATER, Elements::CHAOS), 1.0},
     {std::make_tuple(Elements::WATER, Elements::WATER), 1.0},
     {std::make_tuple(Elements::WATER, Elements::AIR),   0.8},
     {std::make_tuple(Elements::WATER, Elements::FIRE),  1.2},
     {std::make_tuple(Elements::WATER, Elements::EARTH), 0.9},
    
     {std::make_tuple(Elements::AIR, Elements::CHAOS), 1.0},
     {std::make_tuple(Elements::AIR, Elements::WATER), 1.1},
     {std::make_tuple(Elements::AIR, Elements::AIR),   1.0},
     {std::make_tuple(Elements::AIR, Elements::FIRE),  0.8},
     {std::make_tuple(Elements::AIR, Elements::EARTH), 1.1},

     {std::make_tuple(Elements::FIRE, Elements::CHAOS), 1.0},
     {std::make_tuple(Elements::FIRE, Elements::WATER), 0.75},
     {std::make_tuple(Elements::FIRE, Elements::AIR),   1.1},
     {std::make_tuple(Elements::FIRE, Elements::FIRE),  1.0},
     {std::make_tuple(Elements::FIRE, Elements::EARTH), 1.2},

     {std::make_tuple(Elements::EARTH, Elements::CHAOS), 1.0},
     {std::make_tuple(Elements::EARTH, Elements::WATER), 1.0},
     {std::make_tuple(Elements::EARTH, Elements::AIR),   0.9},
     {std::make_tuple(Elements::EARTH, Elements::FIRE),  1.1},
     {std::make_tuple(Elements::EARTH, Elements::EARTH), 1.0}
    };

}

//need to have some centralized notion about what a tower's charcteristics are.
//if each of these essences have 1 or more attributes associated with them, then those
//attributes should map to the tower properties.
//e.g. ares might be % enhanced damage; the tower_properties should have a damage range attribute

template <typename T>
struct range
{
    range(T low_, T high_)
        : low(low_), high(high_)
    {}
    
    range(const range<T>& other)
    {
        low = other.low;
        high = other.high;
    }

	range<T>& operator+=(const range<T>& other)
	{
	    low += other.low;
		high += other.high;
		return *this;
	}

	range<T>& operator+=(T other)
	{
	    low += other;
		high += other;
		return *this;
	}

	range<T>& operator*=(T other)
	{
	    low *= other;
		high *= other;
		return *this;
	}

	range<T>& operator=(const range<T>& other)
    {
        low = other.low;
        high = other.high;
        return *this;
    }

    T low;
    T high;
};

template <typename T>
inline range<T> operator+(range<T> lhs, const range<T>& rhs)
{
    lhs += rhs;
	return lhs;
}

template <typename T>
inline range<T> operator+(range<T> lhs, T other)
{
    lhs += other;
	return lhs;
}

template <typename T>
inline range<T> operator*(range<T> lhs, T other)
{
    lhs *= other;
	return lhs;
}

template <typename T>
inline range<T> operator*(T other, range<T> rhs)
{
    rhs *= other;
	return rhs;
}


/*
what sort of things should a tower have property-wise?    
and how will we do the tiered, randomized "special" attributes/abilities?
i.e. these would be spells and/or other unique effects. We would want to have a lot of these
     in order to have the towers feel unique/different from eachother

*/

#endif



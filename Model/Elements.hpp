#ifndef ELEMENTS_HPP
#define ELEMENTS_HPP

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

#include "RandomUtility.hpp"

/*
 *  We want to have a simplified system here. So plan out the requirements:
 *
 *  Will have each element have strengths and weaknesses versus other elements.
 *  Each element will have a certain attribute affinity, and have higher-tier affinities
 *  water: fast attack speed | slow target's move speed
 *  air: long attack range | critical hit chance
 *  fire: high attack damage | damage over time
 *  earth: lower tower cost | chance to stun
 *
 * ATTACK | DEFENSE
 *  water --> air   
 *  water --> fire
 *  water --> earth
 *
 *  air --> water
 *  air --> fire
 *  air --> earth
 *
 *  fire --> water
 *  fire --> air
 *  fire --> earth
 *
 *  earth --> water
 *  earth --> air
 *  earth --> fire
 *
 *  But then we still have to figure out how to do the tower combination stats;
 *  that is, we'll have some combination of elements, each with their own sets
 *  of weights to use. From this, we have to find the resultant towers' attributes.
 *  This will have to be reflected by the distribution of points it has across 
 *  all of its combining elements. 
 *  The question is, how to represent atributes? They are all pretty different, so
 *  it would be hard to have them be of the same type system
 */

#include <map>
#include <iostream>

namespace ElementTypes
{

template <typename T>
class Element
{
public:
    Element(const T elem_weight)
        : weight(elem_weight)
    {}

    virtual ~Element() {}

    T get_weight() const { return weight; }
protected:
    T weight;
};

/*
 *  D: 1
 *  S; 5
 *  R; 2
 *  C; 3
 */
template <typename T>
class Water : public Element<T>
{
public:    
    explicit Water(const T elem_weight)
        : Element<T>(elem_weight)
    {}
    const static int DMGCoeff = 1;
    const static int SPDCoeff = 5;
    const static int RNGCoeff = 2;
    const static int CSTCoeff = 3;
};


/*
 *  D: 2
 *  S; 3
 *  R; 5 
 *  C; 1
 */
template <typename T>
class Air : public Element<T>
{
public:    
    explicit Air(const T elem_weight)
        : Element<T>(elem_weight)
    {}
    const static int DMGCoeff = 2;
    const static int SPDCoeff = 3;
    const static int RNGCoeff = 5;
    const static int CSTCoeff = 1;
};


/*
 *  D: 5
 *  S; 3
 *  R; 1
 *  C; 2
 */
template <typename T>
class Fire : public Element<T>
{
public:    
    explicit Fire(const T elem_weight)
        : Element<T>(elem_weight)
    {}
    const static int DMGCoeff = 5;
    const static int SPDCoeff = 3;
    const static int RNGCoeff = 1;
    const static int CSTCoeff = 2;
};

/*
 *  D: 3
 *  S; 1
 *  R; 2
 *  C; 5
 */
template <typename T>
class Earth : public Element<T>
{
public:    
    explicit Earth(const T elem_weight)
        : Element<T>(elem_weight)
    {}
    const static int DMGCoeff = 3;
    const static int SPDCoeff = 1;
    const static int RNGCoeff = 2;
    const static int CSTCoeff = 5;
};


}

#endif



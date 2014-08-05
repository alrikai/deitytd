#ifndef TD_ESSENCES_HPP
#define TD_ESSENCES_HPP

#include "TowerDispatcher.hpp"
#include "util/Elements.hpp"

#include "iostream"


/*
 *  would be cool to add other ones in time, i.e. the greek primordial deities,
 *  and the titans, which would have their own essencewords,
 *  see http://en.wikipedia.org/wiki/List_of_Greek_mythological_figures#Major_gods_and_goddess
 */

class essence
{
public:    
    //IMPLEMENT_INDEXABLE_CLASS(essence)
    virtual ~essence() {}
    virtual void apply_modifier(tower_properties& properties) = 0;
};

/*
 *  what properties should these have? --> define some set of properties
 *  that each ones has -- note that they should be constant coefficients
 *  that'll be used to scale with the tier
 */

class aphrodite : public essence
{
public:    
    //IMPLEMENT_INDEXABLE_CLASS(aphrodite)
    void apply_modifier(tower_properties& properties);
};

class apollo : public essence
{
public:    
    //IMPLEMENT_INDEXABLE_CLASS(apollo)
    void apply_modifier(tower_properties& properties);
};

class ares : public essence
{
public:    
    //IMPLEMENT_INDEXABLE_CLASS(ares)
    void apply_modifier(tower_properties& properties);
};

class artemis : public essence
{
public:    
    //IMPLEMENT_INDEXABLE_CLASS(artemis)
    void apply_modifier(tower_properties& properties);
};

class athena : public essence
{
public:    
    //IMPLEMENT_INDEXABLE_CLASS(athena)
    void apply_modifier(tower_properties& properties);
};

class demeter : public essence
{
public:    
    //IMPLEMENT_INDEXABLE_CLASS(demeter)
    void apply_modifier(tower_properties& properties);
};

class dionysus : public essence
{
public:    
    //IMPLEMENT_INDEXABLE_CLASS(dionysus)
    void apply_modifier(tower_properties& properties);
};

class hades : public essence
{
public:    
    //IMPLEMENT_INDEXABLE_CLASS(hades)
    void apply_modifier(tower_properties& properties);
};

class hephaestus : public essence
{
public:    
    //IMPLEMENT_INDEXABLE_CLASS(hephaestus)
    void apply_modifier(tower_properties& properties);
};

class hera : public essence
{
public:    
    //IMPLEMENT_INDEXABLE_CLASS(hera)
    void apply_modifier(tower_properties& properties);
};

class hermes : public essence
{
public:    
    //IMPLEMENT_INDEXABLE_CLASS(hermes)
    void apply_modifier(tower_properties& properties);
};

class hestia : public essence
{
public:    
    //IMPLEMENT_INDEXABLE_CLASS(hestia)
    void apply_modifier(tower_properties& properties);
};

class poseidon : public essence
{
public:    
    //IMPLEMENT_INDEXABLE_CLASS(poseidon)
    void apply_modifier(tower_properties& properties);
};

class zeus : public essence
{
public:    
    //IMPLEMENT_INDEXABLE_CLASS(zeus);
    void apply_modifier(tower_properties& properties);
};

#endif

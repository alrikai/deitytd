#ifndef TD_MONSTER_HPP
#define TD_MONSTER_HPP

#include <tuple>
#include <memory>
#include "ModelUtils.hpp"

/*
 * The monster class -- just a placeholder for now
 */
class Monster
{
public:
    
    Monster(float starting_row, float starting_col)
        : row_pos(starting_row), col_pos(starting_col)
    {
      //placeholder model -- TODO: make some sort of factory arrangement for making the different mobs
      id = CharacterModels::ModelIDs::ogre_S;
    }

    //returned as [row, col]
    std::tuple<float, float> get_position() const
    {
        return std::make_tuple(row_pos, col_pos);
    }

private:
    //normalized positions wrt the map
    float row_pos;
    float col_pos;

		float row_dest;
    float col_dest;

		/*
		 * TODO: determine which stats are needed for the monster type
		 *
		 */
		float health;
		float speed;

    //TODO: need some armor type and amount

		//the character model ID
    CharacterModels::ModelIDs id;
};

template <typename MonsterT, class ... MonsterArgs>
Monster* make_monster(MonsterArgs ... args)
{
  return new MonsterT (std::forward<MonsterArgs>(args) ...);
}

//TODO: have some more detailed factory scheme for creating the different monsters... need to wait on 
//the creation of these other monster types however


#endif

#ifndef TD_MONSTER_HPP
#define TD_MONSTER_HPP

#include <tuple>

/*
 * The monster class -- just a placeholder for now
 */
class Monster
{
public:
    
    Monster(float starting_row, float starting_col)
        : row_pos(starting_row), col_pos(starting_col)
    {}

    //returned as [row, col]
    std::tuple<float, float> get_position() const
    {
        return std::make_tuple(row_pos, col_pos);
    }

private:
    //normalized positions wrt the map
    float row_pos;
    float col_pos;
};

#endif

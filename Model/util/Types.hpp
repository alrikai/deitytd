#ifndef TD_TYPES_HPP
#define TD_TYPES_HPP


template <typename T>
struct Coordinate
{
    Coordinate()
        : col(0), row(0)
    {}

    Coordinate (const T col_, const T row_)
        : col(col_), row(row_)    
    {}

    T col;
    T row;
};

#endif

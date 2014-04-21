#ifndef TD_MAP_TILE_HPP__
#define TD_MAP_TILE_HPP__

struct Coordinate
{
    Coordinate()
        : row(0), col(0)
    {}

    Coordinate (const int row_, const int col_)
        : row(row_), col(col_)    
    {}

    int row;
    int col;
};

struct MapTile
{
    Coordinate loc;
    int width;
    int height;
};

#endif

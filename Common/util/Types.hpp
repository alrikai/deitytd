/* Types.hpp -- part of the DietyTD Model subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */


#ifndef TD_TYPES_HPP
#define TD_TYPES_HPP

#include <memory>
#include <algorithm>
#include <string>

namespace TDHelpers
{
//converts the pre-processor symbol to a std::string
std::string get_TD_path()
{
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

    //generate the buffer for the path string. Make sure it's null-terminated
    const size_t path_len = snprintf(NULL, 0, "%s", TOSTRING(TDPath)) + 1;
    std::unique_ptr<char []> path_buffer = std::unique_ptr<char []> (new char[path_len]);
    std::fill(path_buffer.get(), path_buffer.get() + path_len, 0);
   
    //copy the path string to a C++ string
    snprintf(path_buffer.get(), path_len, "%s", TOSTRING(TDPath));
    std::string td_filepath;
    td_filepath.assign(path_buffer.get());

#undef TOSTRING
#undef STRINGIFY

    return td_filepath;
}

static const std::string& get_basepath()
{
    static std::string path = get_TD_path();
    return path;
}

/*
struct TDSettings
{
    TDSettings()
      : path(get_TD_path())
    {}

    const std::string get() const
    {
        return path;
    }
    const std::string path;
};
//can get the path via TDHelpers::TDbasepath.get()
static const TDSettings TDbasepath;
*/
}

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

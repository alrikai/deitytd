#ifndef TD_CONTROLLER_UTIL_HPP__
#define TD_CONTROLLER_UTIL_HPP__

#include </home/alrik/boost_1_55_0/boost/lockfree/spsc_queue.hpp>

#include <iostream>

namespace ControllerUtil
{
    enum class INPUT_TYPE {A, S, W, D, LArrow, RArrow, UpArrow, DArrow, Esc, LClick, RClick};
    typedef boost::lockfree::spsc_queue<INPUT_TYPE, boost::lockfree::capacity<1024>> ControllerBufferType;


    inline void print_input_type(INPUT_TYPE input)
    {
        switch(input)
        {
            case INPUT_TYPE::LArrow:
                std::cout << "Key L-Arrow" << std::endl;
            break;
            case INPUT_TYPE::RArrow:
                std::cout << "Key R-Arrow" << std::endl;
            break;
            case INPUT_TYPE::UpArrow:
                std::cout << "Key Up-Arrow" << std::endl;
            break;
            case INPUT_TYPE::DArrow:
                std::cout << "Key Down-Arrow" << std::endl;
            break;
            case INPUT_TYPE::A:
                std::cout << "Key A" << std::endl;
            break;
            case INPUT_TYPE::S:
                std::cout << "Key S" << std::endl;
            break;
            case INPUT_TYPE::W:
                std::cout << "Key W" << std::endl;
            break;
            case INPUT_TYPE::D:
                std::cout << "Key D" << std::endl;
            break;
            case INPUT_TYPE::Esc:
                std::cout << "Key Escape" << std::endl;
            break;
            case INPUT_TYPE::LClick:
                std::cout << "Mouse Lclick" << std::endl;
            break;
            case INPUT_TYPE::RClick:
                std::cout << "Mouse Rclick" << std::endl;
            break;
            default:
                std::cout << "Unknown Type" << std::endl;
        };
    }

}

#endif

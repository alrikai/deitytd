#ifndef TD_UTIL_RANDOM_UTIL_HPP
#define TD_UTIL_RANDOM_UTIL_HPP

#include <vector>
#include <random>
#include <algorithm>

namespace Randomize
{
    std::default_random_engine& get_engine()
    {
        static std::random_device rdev{};
        static std::default_random_engine eng{rdev()};
        return eng; 
    }   

    class GaussianRoller
    {
    public:
        GaussianRoller(const int mean, const int variance)
            : dist(mean, variance)
        {}

        void set_roller(int mean, int variance)
        {
            std::normal_distribution<>::param_type new_params {static_cast<double>(mean), static_cast<double>(variance)};
            dist.param(new_params);
        }

        inline int roll_tower_tier()
        {
            return dist(get_engine());
        }
    private:
        std::normal_distribution<> dist;
    };


    class UniformRoller
    {
    public:
        UniformRoller()
        {}
        
        //returns values between [0, weight)  
        inline double get_roll(const double weight)
        {
            return std::generate_canonical<double, std::numeric_limits<double>::digits>(get_engine()) * weight;
        }
    };

}

#endif

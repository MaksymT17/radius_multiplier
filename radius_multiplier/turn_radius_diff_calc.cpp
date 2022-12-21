
#include <iostream>
#include <math.h>

constexpr float radian_multiplier = M_PI / 180;

class TurningMultiplierCalculator
{
public:
    explicit TurningMultiplierCalculator(const float track_length) : m_track_length(track_length)
    {
    }

    float getTurningMultiplier(const int angle)
    {
        const float R1 = m_track_length / sin(((90 - angle) * radian_multiplier) * sin(angle * radian_multiplier));
        const float R2 = R1 + m_track_length;

        return R1 / R2;
    }
    const float m_track_length;
};

int main(int argc, char **argv)
{
    TurningMultiplierCalculator tmc(1.53f); // just for demo) VW jetta Rear track width

    if (argc != 2)
    {
        std::cout << "Left wheel to Right wheel ratio [5 degrees]: " << tmc.getTurningMultiplier(5) << std::endl;
    }
    else{
        std::cout << "Left wheel to Right wheel ratio [5 degrees]: " << tmc.getTurningMultiplier(atoi(argv[1])) << std::endl;
    }
    return 0;
}
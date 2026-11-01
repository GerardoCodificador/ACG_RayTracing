#ifndef HEMISPHERICALSAMPLER_H
#define HEMISPHERICALSAMPLER_H

#include "../core/vector3d.h"

using namespace std;

inline uint32_t s_RndState = 1; //NOTE: NOT THREAD SAFE;


class HemisphericalSampler
{
public:
    HemisphericalSampler();
    Vector3D getSample(const Vector3D &normal) const;
   

    static uint32_t XorShift32()    {
        uint32_t x = s_RndState;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 15;
        s_RndState = x;
        return x;
    }

    static double myRandomDouble() {
        return static_cast<double>(XorShift32()) / static_cast<double>(UINT32_MAX);
    }


};

#endif // HEMISPHERICALSAMPLER_H

#include "arealightsource.h"
#include "../core/hemisphericalsampler.h"

AreaLightSource::AreaLightSource(Square* areaLightsource_) :
    myAreaLightsource(areaLightsource_)
{ }



Vector3D AreaLightSource::getIntensity() const
{
    return myAreaLightsource->getMaterial().getEmissiveRadiance();
}


Vector3D AreaLightSource::sampleLightPosition()   const
{
    // Uniform point on the rectangle: corner + u*v1 + v*v2 with u, v in [0, 1],
    // so every point has the same pdf 1 / getArea()
    double u = HemisphericalSampler::myRandomDouble();
    double v = HemisphericalSampler::myRandomDouble();
    return myAreaLightsource->corner + myAreaLightsource->v1 * u + myAreaLightsource->v2 * v;
}


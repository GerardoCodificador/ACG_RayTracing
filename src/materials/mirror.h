#ifndef MIRRORMATERIAL
#define MIRRORMATERIAL

#include "material.h"

// Perfect specular reflector. It has no BRDF lobe of its own: the integrator
// follows the reflection direction and returns whatever light arrives along it.
class Mirror : public Material
{
public:
    Mirror();

    Vector3D getReflectance(const Vector3D& n, const Vector3D& wo,
        const Vector3D& wi)const ;

    bool hasSpecular() const { return true; }
    bool hasTransmission() const { return false; }
    bool hasDiffuseOrGlossy() const { return false; }
    bool isEmissive() const { return false; }
};
#endif // MIRRORMATERIAL

#include "mirror.h"

#include <iostream>
#include <numbers>
Mirror::Mirror()
{ }

Vector3D Mirror::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {
    return -wo + 2.0 * dot(wo, n) * n;

    
};

double Mirror::getIndexOfRefraction() const
{
    std::cout << "Warning! Calling \"Material::getIndexOfRefraction()\" for a non-transmissive material"
              << std::endl;

    return -1;
}


Vector3D Mirror::getEmissiveRadiance() const
{
    return Vector3D(0.0);
}


Vector3D Mirror::getDiffuseReflectance() const
{
    return rho_d;
}


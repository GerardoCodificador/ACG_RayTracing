#include "transmissive.h"

#include <iostream>
#include <numbers>
Transmissive::Transmissive()
{ }
Transmissive::Transmissive(float ut) {
    Ut = ut;
}
Vector3D Transmissive::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {
    double ut = Ut;
    Vector3D normal = n;
    if (dot(n, wo) < 0.0) {
        ut = 1.0 / ut;
        normal = -normal;
    }

    double n_dot_wo = dot(normal, wo);
    double radicand = 1.0 - (ut * ut) * (1.0 - (n_dot_wo * n_dot_wo));

    if (radicand < 0.0) {
        // Reflexión Interna Total: se comporta como un espejo (reflexión perfecta)
        return -wo + 2.0 * dot(wo, normal) * normal;
    }

    // Retorna el vector de refracción (wt) usando 'ut' en minúscula
    Vector3D wt = -ut * wo + normal * (ut * n_dot_wo - std::sqrt(radicand));
    return wt;
    
};

double Transmissive::getIndexOfRefraction() const
{
    std::cout << "Warning! Calling \"Material::getIndexOfRefraction()\" for a non-transmissive material"
              << std::endl;

    return -1;
}






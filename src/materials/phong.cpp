#include "phong.h"

#include <iostream>
#include <numbers>
Phong::Phong()
{ }

Phong::Phong(Vector3D Kd_, Vector3D Ks_, float alpha_):
rho_d(Kd_), Ks(Ks_), alpha(alpha_){}


Vector3D Phong::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {

    //FILL(...)
	// Ideal reflection of wi about n (Eq. 5)
	Vector3D wr = n * (2.0 * dot(n, wi)) - wi;
	// Clamp so directions past 90 degrees from wr add no highlight (and pow never sees a negative base)
	double specular = std::pow(std::fmax(dot(wo, wr), 0.0), alpha);
	// (alpha + 2) / (2 pi) normalises the lobe so it never reflects more energy than it receives (Eq. 1)
	double normalisation = (alpha + 2.0) / (2.0 * std::numbers::pi);
	return rho_d / std::numbers::pi + Ks * (normalisation * specular);

};

double Phong::getIndexOfRefraction() const
{
    std::cout << "Warning! Calling \"Material::getIndexOfRefraction()\" for a non-transmissive material"
              << std::endl;

    return -1;
}


Vector3D Phong::getEmissiveRadiance() const
{
    return Vector3D(0.0);
}


Vector3D Phong::getDiffuseReflectance() const
{
    return rho_d;
}


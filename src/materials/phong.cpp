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
	float prodescal = n.x * wi.x + n.y * wi.y + n.z * wi.z;
	Vector3D reflectionDirection = 2 * prodescal * n - wi;
	prodescal = reflectionDirection.x * wo.x + reflectionDirection.y * wo.y + reflectionDirection.z * wo.z;
	prodescal = std::pow(prodescal, alpha);
	return rho_d/std::numbers::pi + Ks * prodescal;

    return Vector3D(0.0);

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


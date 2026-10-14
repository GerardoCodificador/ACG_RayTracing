#include "transmissive.h"

Transmissive::Transmissive() : mu_t(1.0)
{ }

Transmissive::Transmissive(double mu_t_) : mu_t(mu_t_)
{ }

Vector3D Transmissive::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {
    // Light only passes straight through along the refracted direction,
    // which the integrator traces itself.
    return Vector3D(0.0);
};

double Transmissive::getIndexOfRefraction() const
{
    return mu_t;
}

#include "mirror.h"

Mirror::Mirror()
{ }

Vector3D Mirror::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {
    // A perfect mirror reflects only along the ideal direction (a delta BRDF),
    // which the integrator handles by tracing a reflected ray instead.
    return Vector3D(0.0);
};

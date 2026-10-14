#ifndef TRANSMISSIVEMATERIAL
#define TRANSMISSIVEMATERIAL

#include "material.h"

// Perfect specular transmitter (e.g. glass). Like Mirror it has no BRDF lobe:
// the integrator bends the ray using the ratio of refractive indices.
class Transmissive : public Material
{
public:
    Transmissive();
    Transmissive(double mu_t);
    Vector3D getReflectance(const Vector3D& n, const Vector3D& wo,
        const Vector3D& wi)const ;

    bool hasSpecular() const { return false; }
    bool hasTransmission() const { return true; }
    bool hasDiffuseOrGlossy() const { return false; }
    bool isEmissive() const { return false; }

    double getIndexOfRefraction() const;


private:
    double mu_t;  // eta1 / eta2, ratio of refractive indices (outside / inside)
};
#endif // TRANSMISSIVEMATERIAL

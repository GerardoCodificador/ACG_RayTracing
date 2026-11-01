#ifndef AREADIRECTINTEGRATOR_H
#define AREADIRECTINTEGRATOR_H

#include "shader.h"
#include "../core/intersection.h"


class AreaDirectIntegrator : public Shader
{
public:
    AreaDirectIntegrator();
    AreaDirectIntegrator(Vector3D bgColor_, double ambient_, size_t maxDepth_);

    Vector3D computeColor(const Ray &r,
                             const std::vector<Shape*> &objList,
                             const std::vector<LightSource*> &lsList) const;

    // Direct light at a hit point, estimated with one random point per area light (Eq. 8).
    // Shared with the NEE integrator.
    static Vector3D directLight(const Intersection &its, const Vector3D &wo,
                                const std::vector<Shape*> &objList,
                                const std::vector<LightSource*> &lsList);

private:
    double ambient;   // I_a: L_ind is approximated by I_a * k_d
    size_t maxDepth;  // max number of specular bounces
};

#endif // AREADIRECTINTEGRATOR_H

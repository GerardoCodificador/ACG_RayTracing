#ifndef HEMISPHERICALDIRECTINTEGRATOR_H
#define HEMISPHERICALDIRECTINTEGRATOR_H

#include "shader.h"
#include "../core/hemisphericalsampler.h"


class HemisphericalDirectIntegrator : public Shader
{
public:
    HemisphericalDirectIntegrator();
    HemisphericalDirectIntegrator(Vector3D bgColor_, double ambient_, size_t maxDepth_);

    Vector3D computeColor(const Ray &r,
                             const std::vector<Shape*> &objList,
                             const std::vector<LightSource*> &lsList) const;

private:
    double ambient;   // I_a: L_ind is approximated by I_a * k_d
    size_t maxDepth;  // max number of specular bounces
    HemisphericalSampler sampler;
};

#endif // HEMISPHERICALDIRECTINTEGRATOR_H

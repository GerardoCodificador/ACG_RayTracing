#ifndef NEEINTEGRATOR_H
#define NEEINTEGRATOR_H

#include "shader.h"
#include "../core/hemisphericalsampler.h"


class NEEIntegrator : public Shader
{
public:
    NEEIntegrator();
    NEEIntegrator(Vector3D bgColor_, size_t maxDepth_);

    Vector3D computeColor(const Ray &r,
                             const std::vector<Shape*> &objList,
                             const std::vector<LightSource*> &lsList) const;

private:
    Vector3D reflectedRadiance(const Intersection &its, const Ray &r,
                               const std::vector<Shape*> &objList,
                               const std::vector<LightSource*> &lsList) const;
    Vector3D indirectRadiance(const Intersection &its, const Vector3D &wo, size_t depth,
                              const std::vector<Shape*> &objList,
                              const std::vector<LightSource*> &lsList) const;

    size_t maxDepth;  // max number of bounces of a path
    HemisphericalSampler sampler;
};

#endif // NEEINTEGRATOR_H

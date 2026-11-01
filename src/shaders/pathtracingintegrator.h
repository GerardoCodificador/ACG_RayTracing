#ifndef PATHTRACINGINTEGRATOR_H
#define PATHTRACINGINTEGRATOR_H

#include "shader.h"
#include "../core/hemisphericalsampler.h"


class PathTracingIntegrator : public Shader
{
public:
    PathTracingIntegrator();
    PathTracingIntegrator(Vector3D bgColor_, size_t maxDepth_);

    Vector3D computeColor(const Ray &r,
                             const std::vector<Shape*> &objList,
                             const std::vector<LightSource*> &lsList) const;

private:
    size_t maxDepth;  // max number of bounces of a path
    HemisphericalSampler sampler;
};

#endif // PATHTRACINGINTEGRATOR_H

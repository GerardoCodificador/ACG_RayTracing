#ifndef WHITTEDINTEGRATOR_H
#define WHITTEDINTEGRATOR_H

#include "shader.h"


class WhittedIntegrator : public Shader
{
public:
    WhittedIntegrator();
    WhittedIntegrator(Vector3D bgColor_, double ambient_, size_t maxDepth_);

    Vector3D computeColor(const Ray &r,
                             const std::vector<Shape*> &objList,
                             const std::vector<LightSource*> &lsList) const;

private:
    double ambient;   // a_t in Eq. 7
    size_t maxDepth;  // max number of specular bounces before giving up
};

#endif // WHITTEDINTEGRATOR_H

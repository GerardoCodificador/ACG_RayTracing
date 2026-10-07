#ifndef WHITESHADER_H
#define WHITESHADER_H

#include "shader.h"


class WhiteShader : public Shader
{
public:
    WhiteShader();
    WhiteShader(Vector3D color_, double maxDist_, Vector3D bgColor_);

    Vector3D computeColor(const Ray &r,
                             const std::vector<Shape*> &objList,
                             const std::vector<LightSource*> &lsList) const;

private:
    double ambient=0.1;
    double maxDist;
    Vector3D color;
};

#endif // DEPTHSHADER_H

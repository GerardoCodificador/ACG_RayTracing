#include "normalshader.h"

#include "../core/utils.h"

NormalShader::NormalShader()
{ }

NormalShader::NormalShader(Vector3D bgColor_) :
    Shader(bgColor_)
{ }

Vector3D NormalShader::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
	//TASK4
	Intersection its;
	if (Utils::getClosestIntersection(r, objList, its))
	{
		// Eq. 3: map each normal component from [-1, 1] to a colour in [0, 1]
		return (its.normal + Vector3D(1.0)) / 2;
	}

	return bgColor;
}

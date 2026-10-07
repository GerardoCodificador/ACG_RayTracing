#include "normalshader.h"

#include "../core/utils.h"

NormalShader::NormalShader() :
    color(Vector3D(0, 1, 0))
{ }

NormalShader::NormalShader(Vector3D hitColor_, double maxDist_, Vector3D bgColor_) :
    Shader(bgColor_), maxDist(maxDist_), color(hitColor_)
{ }

Vector3D NormalShader::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
    //(FILL..)
	//TASK4
    //if..
	Intersection its;
	if (Utils::getClosestIntersection(r, objList,its))
	{
		Vector3D normal = its.normal;
			Vector3D intersectionColor = (color+normal)/2;
		return intersectionColor;
	}
    //else...

    return bgColor;
}

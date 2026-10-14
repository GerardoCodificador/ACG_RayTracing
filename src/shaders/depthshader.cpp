#include "depthshader.h"

#include "../core/utils.h"

DepthShader::DepthShader() :
    color(Vector3D(0, 1, 0))
{ }

DepthShader::DepthShader(Vector3D hitColor_, double maxDist_, Vector3D bgColor_) :
    Shader(bgColor_), maxDist(maxDist_), color(hitColor_)
{ }

Vector3D DepthShader::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
    //(FILL..)
	//TASK3
    //if..
	Intersection its;
	if (Utils::getClosestIntersection(r, objList,its))
	{
		float dist = (its.itsPoint - r.o).length();
		// Eq. 2: fade the hit colour linearly to black at maxDist
		Vector3D intersectionColor = color * std::fmax(1.0 - dist / maxDist, 0.0);
		return intersectionColor;
	}
    //else...

    return bgColor;
}

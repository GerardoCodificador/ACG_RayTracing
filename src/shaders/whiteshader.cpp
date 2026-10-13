#include "whiteshader.h"

#include "../core/utils.h"

WhiteShader::WhiteShader() :
    color(Vector3D(0, 1, 0))
{ }

WhiteShader::WhiteShader(Vector3D hitColor_, double maxDist_, Vector3D bgColor_) :
    Shader(bgColor_), maxDist(maxDist_), color(hitColor_)
{ }

Vector3D WhiteShader::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
    //(FILL..)
	//TASK5
    //if..
	Intersection its;
	if (Utils::getClosestIntersection(r, objList,its))
	{
		if (its.shape->getMaterial().hasDiffuseOrGlossy()) {
			Vector3D rayinverse = -r.d;
			for (int i = 0; i < lsList.size(); i++) {
				LightSource* light = lsList[i];
				float dist = (its.itsPoint - light->sampleLightPosition()).length();
				Vector3D wos = (light->sampleLightPosition() - its.itsPoint).normalized();
				Intersection its2;
				Ray shadowRay(light->sampleLightPosition(), -wos);
				int lightcolision = 0;
				if (Utils::getClosestIntersection(shadowRay, objList, its2)) {
					if ((its2.itsPoint - shadowRay.o).length() < dist - 0.0001f) {
						lightcolision = 0;
					}
					else {
						lightcolision = 1;
					}

				}


				Vector3D normal = its.normal;

				Vector3D intersectionColor = (color + normal) / 2;
				return its.shape->getMaterial().getDiffuseReflectance() * ambient + its.shape->getMaterial().getReflectance(normal, wos, rayinverse) * light->getIntensity() * (dot(normal, wos)) * lightcolision;


			}
		}
		else if (its.shape->getMaterial().hasSpecular()) {
			Vector3D normal = its.normal;

			Vector3D wr=its.shape->getMaterial().getReflectance(normal, -r.d, Vector3D(0));
			Ray reflected(its.itsPoint, wr,r.depth+1);
			return WhiteShader::computeColor(reflected, objList, lsList);



		}
		else if (its.shape->getMaterial().hasTransmission()) {

			Vector3D normal = its.normal;
			Vector3D wr = its.shape->getMaterial().getReflectance(normal, -r.d, Vector3D(0));
			Ray reflected(its.itsPoint , wr, r.depth + 1);
			reflected.depth = r.depth + 1;
			return WhiteShader::computeColor(reflected, objList, lsList);
		}
		
	}
    //else...

    return bgColor;
}

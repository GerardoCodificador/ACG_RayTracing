#include "whittedintegrator.h"

#include "../core/utils.h"

WhittedIntegrator::WhittedIntegrator() :
	ambient(0.1), maxDepth(10)
{ }

WhittedIntegrator::WhittedIntegrator(Vector3D bgColor_, double ambient_, size_t maxDepth_) :
	Shader(bgColor_), ambient(ambient_), maxDepth(maxDepth_)
{ }

Vector3D WhittedIntegrator::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
	//TASK5
	Intersection its;
	if (!Utils::getClosestIntersection(r, objList, its))
		return bgColor;

	// Stop runaway recursion (e.g. two mirrors facing each other)
	if (r.depth > maxDepth)
		return Vector3D(0.0);

	const Material& material = its.shape->getMaterial();
	Vector3D normal = its.normal;
	Vector3D wo = -r.d;

	if (material.hasDiffuseOrGlossy()) {
		// Eq. 7: ambient term plus the direct contribution of every visible point light
		Vector3D color = material.getDiffuseReflectance() * ambient;
		for (size_t i = 0; i < lsList.size(); i++) {
			const LightSource* light = lsList[i];
			Vector3D toLight = light->sampleLightPosition() - its.itsPoint;
			double distToLight = toLight.length();
			Vector3D wi = toLight.normalized();

			// Visibility V(x): anything between the point and the light blocks it
			Ray shadowRay(its.itsPoint, wi, 0, Epsilon, distToLight);
			if (Utils::hasIntersection(shadowRay, objList))
				continue;

			double cosTheta = std::fmax(dot(normal, wi), 0.0);
			color += light->getIntensity() * material.getReflectance(normal, wo, wi) * cosTheta;
		}
		return color;
	}
	else if (material.hasSpecular()) {
		// Perfect mirror: the colour is whatever arrives from the reflected direction
		Vector3D wr = Utils::computeReflectionDirection(r.d, normal);
		Ray reflected(its.itsPoint, wr, r.depth + 1);
		return computeColor(reflected, objList, lsList);
	}
	else if (material.hasTransmission()) {
		Vector3D wt = material.getReflectance(normal, wo, Vector3D(0));
		Ray refracted(its.itsPoint, wt, r.depth + 1);
		return computeColor(refracted, objList, lsList);
	}

	return bgColor;
}

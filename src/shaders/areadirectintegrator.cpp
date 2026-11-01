#include "areadirectintegrator.h"

#include "../core/utils.h"

AreaDirectIntegrator::AreaDirectIntegrator() :
	ambient(0.1), maxDepth(10)
{ }

AreaDirectIntegrator::AreaDirectIntegrator(Vector3D bgColor_, double ambient_, size_t maxDepth_) :
	Shader(bgColor_), ambient(ambient_), maxDepth(maxDepth_)
{ }

Vector3D AreaDirectIntegrator::directLight(const Intersection &its, const Vector3D &wo,
                                           const std::vector<Shape*> &objList,
                                           const std::vector<LightSource*> &lsList)
{
	const Material& material = its.shape->getMaterial();
	Vector3D normal = its.normal;
	Vector3D color(0.0);

	for (size_t i = 0; i < lsList.size(); i++) {
		const LightSource* light = lsList[i];
		// Uniform point y on the light, pdf = 1 / area
		Vector3D y = light->sampleLightPosition();
		Vector3D toLight = y - its.itsPoint;
		double dist2 = toLight.lengthSq();
		double dist = std::sqrt(dist2);
		Vector3D wi = toLight / dist;

		// Geometric term G(x, y): cosines at both ends, inverse-square falloff (Eq. 7)
		double cosX = dot(normal, wi);
		double cosY = dot(light->getNormal(), -wi);
		if (cosX <= 0.0 || cosY <= 0.0)
			continue;

		// Visibility V(x, y). Stop just short of y: the light lies on the ceiling plane,
		// which would otherwise block the ray at exactly the same distance
		Ray shadowRay(its.itsPoint, wi, 0, Epsilon, dist - Epsilon);
		if (Utils::hasIntersection(shadowRay, objList))
			continue;

		double G = cosX * cosY / dist2;
		color += light->getIntensity() * material.getReflectance(normal, wo, wi) * (G * light->getArea());
	}
	return color;
}

Vector3D AreaDirectIntegrator::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
	Intersection its;
	if (!Utils::getClosestIntersection(r, objList, its))
		return bgColor;

	if (r.depth > maxDepth)
		return Vector3D(0.0);

	const Material& material = its.shape->getMaterial();
	Vector3D normal = its.normal;
	Vector3D wo = -r.d;

	// Looking straight at a light: only its emitting side is bright
	if (material.isEmissive())
		return dot(normal, wo) > 0.0 ? material.getEmissiveRadiance() : Vector3D(0.0);

	if (material.hasSpecular()) {
		Ray reflected(its.itsPoint, Utils::computeReflectionDirection(r.d, normal), r.depth + 1);
		return computeColor(reflected, objList, lsList);
	}
	if (material.hasTransmission()) {
		Ray refracted(its.itsPoint, Utils::computeTransmissionDirection(r.d, normal, material.getIndexOfRefraction()), r.depth + 1);
		return computeColor(refracted, objList, lsList);
	}

	// Ambient stand-in for indirect light, plus direct light sampled on the lights' area
	return material.getDiffuseReflectance() * ambient + directLight(its, wo, objList, lsList);
}

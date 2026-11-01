#include "hemisphericaldirectintegrator.h"

#include <numbers>
#include "../core/utils.h"

HemisphericalDirectIntegrator::HemisphericalDirectIntegrator() :
	ambient(0.1), maxDepth(10)
{ }

HemisphericalDirectIntegrator::HemisphericalDirectIntegrator(Vector3D bgColor_, double ambient_, size_t maxDepth_) :
	Shader(bgColor_), ambient(ambient_), maxDepth(maxDepth_)
{ }

Vector3D HemisphericalDirectIntegrator::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
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

	// Indirect light approximated by a constant ambient term
	Vector3D color = material.getDiffuseReflectance() * ambient;

	// Eq. 5 with one sample: pick a uniform direction on the hemisphere (pdf = 1 / 2pi)
	// and keep it only if it happens to hit the emitting side of a light
	Vector3D wi = sampler.getSample(normal);
	Ray sampleRay(its.itsPoint, wi);
	Intersection lightIts;
	if (Utils::getClosestIntersection(sampleRay, objList, lightIts)) {
		const Material& hitMaterial = lightIts.shape->getMaterial();
		if (hitMaterial.isEmissive() && dot(lightIts.normal, -wi) > 0.0) {
			double pdf = 1.0 / (2.0 * std::numbers::pi);
			color += hitMaterial.getEmissiveRadiance() * material.getReflectance(normal, wo, wi) * (dot(normal, wi) / pdf);
		}
	}
	return color;
}

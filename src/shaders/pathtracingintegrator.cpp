#include "pathtracingintegrator.h"

#include <numbers>
#include "../core/utils.h"

PathTracingIntegrator::PathTracingIntegrator() :
	maxDepth(5)
{ }

PathTracingIntegrator::PathTracingIntegrator(Vector3D bgColor_, size_t maxDepth_) :
	Shader(bgColor_), maxDepth(maxDepth_)
{ }

// Algorithm 1: radiance along r = emission at the hit + one random continuation of the path
Vector3D PathTracingIntegrator::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
	Intersection its;
	if (!Utils::getClosestIntersection(r, objList, its))
		return bgColor;

	const Material& material = its.shape->getMaterial();
	Vector3D normal = its.normal;
	Vector3D wo = -r.d;

	// L_e: only the emitting side of a light is bright
	Vector3D color(0.0);
	if (material.isEmissive() && dot(normal, wo) > 0.0)
		color = material.getEmissiveRadiance();

	if (r.depth >= maxDepth)
		return color;

	if (material.hasSpecular()) {
		// Delta BRDF: the only direction that contributes is the mirror one, with weight 1
		Ray reflected(its.itsPoint, Utils::computeReflectionDirection(r.d, normal), r.depth + 1);
		return color + computeColor(reflected, objList, lsList);
	}
	if (material.hasTransmission()) {
		Ray refracted(its.itsPoint, Utils::computeTransmissionDirection(r.d, normal, material.getIndexOfRefraction()), r.depth + 1);
		return color + computeColor(refracted, objList, lsList);
	}

	// Diffuse/glossy: continue in a uniform random direction (pdf = 1 / 2pi) and weight
	// the light it brings back by BRDF * cosine / pdf
	Vector3D wi = sampler.getSample(normal);
	Ray bounce(its.itsPoint, wi, r.depth + 1);
	double pdf = 1.0 / (2.0 * std::numbers::pi);
	color += computeColor(bounce, objList, lsList) * material.getReflectance(normal, wo, wi) * (dot(normal, wi) / pdf);
	return color;
}

#include "neeintegrator.h"

#include <numbers>
#include "areadirectintegrator.h"
#include "../core/utils.h"

NEEIntegrator::NEEIntegrator() :
	maxDepth(5)
{ }

NEEIntegrator::NEEIntegrator(Vector3D bgColor_, size_t maxDepth_) :
	Shader(bgColor_), maxDepth(maxDepth_)
{ }

// Algorithm 2: L = L_e + L_r at the first hit
Vector3D NEEIntegrator::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
	Intersection its;
	if (!Utils::getClosestIntersection(r, objList, its))
		return bgColor;

	const Material& material = its.shape->getMaterial();
	Vector3D wo = -r.d;

	Vector3D color(0.0);
	if (material.isEmissive() && dot(its.normal, wo) > 0.0)
		color = material.getEmissiveRadiance();

	return color + reflectedRadiance(its, wo, r.depth, objList, lsList);
}

// L_r = L_dir (sampled on the lights) + L_ind (one random bounce). Never adds L_e:
// light reached by a bounce was already counted by the direct term of the previous point.
Vector3D NEEIntegrator::reflectedRadiance(const Intersection &its, const Vector3D &wo, size_t depth,
                                          const std::vector<Shape*> &objList,
                                          const std::vector<LightSource*> &lsList) const
{
	// Phong materials only (Task 4.3.2)
	if (!its.shape->getMaterial().hasDiffuseOrGlossy())
		return Vector3D(0.0);

	return AreaDirectIntegrator::directLight(its, wo, objList, lsList)
		+ indirectRadiance(its, wo, depth, objList, lsList);
}

Vector3D NEEIntegrator::indirectRadiance(const Intersection &its, const Vector3D &wo, size_t depth,
                                         const std::vector<Shape*> &objList,
                                         const std::vector<LightSource*> &lsList) const
{
	depth = depth + 1;
	if (depth >= maxDepth)
		return Vector3D(0.0);

	// Uniform direction on the hemisphere, pdf = 1 / 2pi
	Vector3D wi = sampler.getSample(its.normal);
	Ray bounce(its.itsPoint, wi, depth);
	Intersection next;
	if (!Utils::getClosestIntersection(bounce, objList, next))
		return Vector3D(0.0);

	double pdf = 1.0 / (2.0 * std::numbers::pi);
	return reflectedRadiance(next, -wi, depth, objList, lsList)
		* its.shape->getMaterial().getReflectance(its.normal, wo, wi) * (dot(its.normal, wi) / pdf);
}

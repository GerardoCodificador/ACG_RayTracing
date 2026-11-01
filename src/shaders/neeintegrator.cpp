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

// Algorithm 2: L = L_e + L_r. L_e is only added when the ray says so (see Ray::countEmission)
Vector3D NEEIntegrator::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
	Intersection its;
	if (!Utils::getClosestIntersection(r, objList, its))
		return bgColor;

	const Material& material = its.shape->getMaterial();

	Vector3D color(0.0);
	if (r.countEmission && material.isEmissive() && dot(its.normal, -r.d) > 0.0)
		color = material.getEmissiveRadiance();

	return color + reflectedRadiance(its, r, objList, lsList);
}

Vector3D NEEIntegrator::reflectedRadiance(const Intersection &its, const Ray &r,
                                          const std::vector<Shape*> &objList,
                                          const std::vector<LightSource*> &lsList) const
{
	const Material& material = its.shape->getMaterial();
	Vector3D wo = -r.d;
	size_t depth = r.depth + 1;

	// Mirror and glass: follow the single specular direction. Direct light sampling can
	// never reach a light through them, so the next hit must count its emission
	if (material.hasSpecular() || material.hasTransmission()) {
		if (depth >= maxDepth)
			return Vector3D(0.0);
		Vector3D dir = material.hasSpecular()
			? Utils::computeReflectionDirection(r.d, its.normal)
			: Utils::computeTransmissionDirection(r.d, its.normal, material.getIndexOfRefraction());
		Ray specular(its.itsPoint, dir, depth);
		specular.countEmission = true;
		return computeColor(specular, objList, lsList);
	}

	// Diffuse/glossy: L_r = L_dir (sampled on the lights) + L_ind (one random bounce)
	return AreaDirectIntegrator::directLight(its, wo, objList, lsList)
		+ indirectRadiance(its, wo, r.depth, objList, lsList);
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
	// Light reached by this bounce was already counted by the direct term above
	bounce.countEmission = false;

	double pdf = 1.0 / (2.0 * std::numbers::pi);
	return computeColor(bounce, objList, lsList)
		* its.shape->getMaterial().getReflectance(its.normal, wo, wi) * (dot(its.normal, wi) / pdf);
}

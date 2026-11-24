#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

#include "../framework/camera.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"

namespace easyVDB { class OpenVDBReader; }

class Material {
public:

	Shader* shader = NULL;
	Texture* texture = NULL;
	glm::vec4 color;

	virtual void setUniforms(Camera* camera, glm::mat4 model) = 0;
	virtual void render(Mesh* mesh, glm::mat4 model, Camera* camera) = 0;
	virtual void renderInMenu() = 0;
};

class FlatMaterial : public Material {
public:

	FlatMaterial(glm::vec4 color = glm::vec4(1.f));
	~FlatMaterial();

	void setUniforms(Camera* camera, glm::mat4 model);
	void render(Mesh* mesh, glm::mat4 model, Camera* camera);
	void renderInMenu();
};

class WireframeMaterial : public FlatMaterial {
public:
	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, glm::mat4 model, Camera* camera);
};
class VolumeMaterial : public FlatMaterial {
public:
	int Task;
	float stepsize;
	float absortion_coefficient;
	float noisefactor;
	glm::vec4& background_color;
	glm::vec3 emitedLight;

	// Lab 4 Task 1: the density comes from 3D noise or from a VDB file stored in this->texture
	int densitySource = 0;    // 0 = 3D noise, 1 = VDB file
	int vdbFileIndex = 0;     // file selected in the GUI
	int loadedVdbIndex = -1;  // file currently in the texture (-1 = none)

	// Lab 4 Task 2: in-scattering of light from the scene's light source
	float scattering_coefficient = 10.0f; // mu_s, scaled by the density like mu_a
	int lightSteps = 16;                  // steps of the second ray march, towards the light

	// Lab 4 Task 3: Henyey-Greenstein asymmetry, -1 = back-scattering, 0 = isotropic, 1 = forward
	float g = 0.0f;

	VolumeMaterial(float ac,glm::vec4& bgcol);
	~VolumeMaterial();

	void loadVDB(std::string file_path);
	void estimate3DTexture(easyVDB::OpenVDBReader* vdbReader);

	void render(Mesh* mesh, glm::mat4 model, Camera* camera);
	void renderInMenu();
	void setUniforms(Camera* camera, glm::mat4 model, Mesh* mesh);
};

class StandardMaterial : public Material {
public:

	bool first_pass = false;

	bool show_normals = false;
	Shader* base_shader = NULL;
	Shader* normal_shader = NULL;

	StandardMaterial(glm::vec4 color = glm::vec4(1.f));
	~StandardMaterial();

	void setUniforms(Camera* camera, glm::mat4 model);
	void render(Mesh* mesh, glm::mat4 model, Camera* camera);
	void renderInMenu();
};
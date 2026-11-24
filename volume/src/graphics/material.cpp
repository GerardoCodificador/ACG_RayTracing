#include "material.h"

#include "application.h"

#include <istream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>

#include "openvdbReader.h"
#include "grid.h"
#include "bbox.h"

// VDB files the user can pick in the GUI (Lab 4 Task 1)
static const char* vdb_files[] = { "res/volumes/bunny_cloud.vdb", "res/volumes/smoke.vdb" };
static const char* vdb_names[] = { "Bunny cloud", "Smoke" };

FlatMaterial::FlatMaterial(glm::vec4 color)
{
	this->color = color;
	this->shader = Shader::Get("res/shaders/basic.vs", "res/shaders/flat.fs");
}

FlatMaterial::~FlatMaterial() { }

void FlatMaterial::setUniforms(Camera* camera, glm::mat4 model)
{
	// Upload node uniforms
	this->shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	this->shader->setUniform("u_camera_position", camera->eye);
	this->shader->setUniform("u_model", model);

	this->shader->setUniform("u_color", this->color);
}

void FlatMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	if (mesh && this->shader) {
		// Enable shader
		this->shader->enable();

		// Upload uniforms
		setUniforms(camera, model);

		// Do the draw call
		mesh->render(GL_TRIANGLES);

		this->shader->disable();
	}
}

void FlatMaterial::renderInMenu()
{
	ImGui::Text("Material Type: %s", std::string("Flat").c_str());

	ImGui::ColorEdit3("Color", (float*)&this->color);
}

WireframeMaterial::WireframeMaterial()
{
	this->color = glm::vec4(1.f);
	this->shader = Shader::Get("res/shaders/basic.vs", "res/shaders/flat.fs");
}


WireframeMaterial::~WireframeMaterial() { }

VolumeMaterial::VolumeMaterial(float ac, glm::vec4& bgcol) : background_color(bgcol) // <-- Vincula la referencia aquí
{	
	this->noisefactor = 0.0f;
	this->Task = 3; // Lab 4: start on the full model with scattering
	this->stepsize = 0.01f;
	this->absortion_coefficient = ac;
	this->color = glm::vec4(1.f);
	this->shader = Shader::Get("res/shaders/basic.vs", "res/shaders/volume_scattering.fs");
	this->emitedLight = glm::vec3(0.0f); // no emission by default (Fig. 1 of Lab 4 has none)
}


VolumeMaterial::~VolumeMaterial() { }
void VolumeMaterial::setUniforms(Camera* camera, glm::mat4 model, Mesh* mesh)
{
	// 1. Límites transformados de la caja
	glm::vec3 localMin = mesh->aabb_min;
	glm::vec3 localMax = mesh->aabb_max;

	glm::vec3 worldMin = glm::vec3(model * glm::vec4(localMin, 1.0f));
	glm::vec3 worldMax = glm::vec3(model * glm::vec4(localMax, 1.0f));

	glm::vec3 finalMin = glm::min(worldMin, worldMax);
	glm::vec3 finalMax = glm::max(worldMin, worldMax);

	this->shader->setUniform("u_boxmin", finalMin);
	this->shader->setUniform("u_boxmax", finalMax);

	// 2. Matrices y cámara
	this->shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	this->shader->setUniform("u_camera_position", camera->eye);
	this->shader->setUniform("u_model", model);

	// 3. Parámetros del volumen (Asegúrate de inicializar estos valores en el constructor del material)
	this->shader->setUniform("u_absortion_coefficient", this->absortion_coefficient);
	this->shader->setUniform("u_background_color", this->background_color);

	this->shader->setUniform("u_task", this->Task+1 );
	this->shader->setUniform("u_step_size", this->stepsize);

	this->shader->setUniform("u_emitted_color", this->emitedLight);
	this->shader->setUniform("u_noise_factor", this->noisefactor);

	// Lab 4 Task 1: where the density comes from, and what the shader needs to turn a world
	// position into texture coordinates (world -> local cube space -> [0,1]^3)
	this->shader->setUniform("u_density_source", this->densitySource);

	// Lab 4 Task 2: scattering parameters (the light's own uniforms are set in render())
	this->shader->setUniform("u_scattering_coefficient", this->scattering_coefficient);
	this->shader->setUniform("u_light_steps", this->lightSteps);
	this->shader->setUniform("u_g", this->g);
	this->shader->setUniform("u_inverse_model", glm::inverse(model));
	this->shader->setUniform("u_local_boxmin", mesh->aabb_min);
	this->shader->setUniform("u_local_boxmax", mesh->aabb_max);
	if (this->texture)
		this->shader->setUniform("u_volume_texture", this->texture, 0);
}

// Read a VDB file with easyVDB and store its density grid in this->texture (from the course gist)
void VolumeMaterial::loadVDB(std::string file_path)
{
	if (!std::filesystem::exists(file_path)) {
		std::cout << "VDB file not found: " << file_path << std::endl;
		return;
	}

	easyVDB::OpenVDBReader* vdbReader = new easyVDB::OpenVDBReader();
	vdbReader->read(file_path);

	// now, read the grid from the vdbReader and store the data in a 3D texture
	estimate3DTexture(vdbReader);
	// Not deleted: easyVDB's reader does not clean up safely (deleting it crashes), as in the course gist
}

// Resample the VDB grid into a resolution^3 block of densities and upload it as a 3D texture
void VolumeMaterial::estimate3DTexture(easyVDB::OpenVDBReader* vdbReader)
{
	int resolution = 128;
	float radius = 2.0;

	int convertedGrids = 0;
	int convertedVoxels = 0;

	int totalGrids = vdbReader->gridsSize;
	int totalVoxels = totalGrids * pow(resolution, 3);

	float resolutionInv = 1.0f / resolution;
	int resolutionPow2 = pow(resolution, 2);
	int resolutionPow3 = pow(resolution, 3);

	// read all grids data and convert to texture
	for (unsigned int i = 0; i < totalGrids; i++) {
		easyVDB::Grid& grid = vdbReader->grids[i];
		float* data = new float[resolutionPow3];
		memset(data, 0, sizeof(float) * resolutionPow3);

		// Bbox
		easyVDB::Bbox bbox = easyVDB::Bbox();
		bbox = grid.getPreciseWorldBbox();
		glm::vec3 target = bbox.getCenter();
		glm::vec3 size = bbox.getSize();
		glm::vec3 step = size * resolutionInv;

		grid.transform->applyInverseTransformMap(step);
		target = target - (size * 0.5f);
		grid.transform->applyInverseTransformMap(target);
		target = target + (step * 0.5f);

		int x = 0;
		int y = 0;
		int z = 0;

		for (unsigned int j = 0; j < resolutionPow3; j++) {
			int baseX = x;
			int baseY = y;
			int baseZ = z;
			int baseIndex = baseX + baseY * resolution + baseZ * resolutionPow2;

			float value = grid.getValue(target);

			int cellBleed = radius;

			if (cellBleed) {
				for (int sx = -cellBleed; sx < cellBleed; sx++) {
					for (int sy = -cellBleed; sy < cellBleed; sy++) {
						for (int sz = -cellBleed; sz < cellBleed; sz++) {
							if (x + sx < 0.0 || x + sx >= resolution ||
								y + sy < 0.0 || y + sy >= resolution ||
								z + sz < 0.0 || z + sz >= resolution) {
								continue;
							}

							int targetIndex = baseIndex + sx + sy * resolution + sz * resolutionPow2;

							float offset = std::max(0.0, std::min(1.0, 1.0 - std::hypot(sx, sy, sz) / (radius / 2.0)));
							float dataValue = offset * value * 255.f;

							data[targetIndex] += dataValue;
							data[targetIndex] = std::min((float)data[targetIndex], 255.f);
						}
					}
				}
			}
			else {
				float dataValue = value * 255.f;

				data[baseIndex] += dataValue;
				data[baseIndex] = std::min((float)data[baseIndex], 255.f);
			}

			convertedVoxels++;

			if (z >= resolution) {
				break;
			}

			x++;
			target.x += step.x;

			if (x >= resolution) {
				x = 0;
				target.x -= step.x * resolution;

				y++;
				target.y += step.y;
			}

			if (y >= resolution) {
				y = 0;
				target.y -= step.y * resolution;

				z++;
				target.z += step.z;
			}
		}

		// The values above are in [0, 255], but float data uploaded to a GL_R8 texture is
		// clamped to [0, 1]: rescale first, otherwise every non-empty voxel becomes density 1
		for (int k = 0; k < resolutionPow3; k++)
			data[k] /= 255.f;

		// now we create the texture with the data
		if (this->texture) delete this->texture;
		this->texture = new Texture();
		this->texture->create3D(resolution, resolution, resolution, GL_RED, GL_FLOAT, false, data, GL_R8);
		delete[] data;
	}
}

void VolumeMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	// Lab 4 Task 1: load the selected VDB file the first time it is needed (takes a moment)
	if (this->densitySource == 1 && this->loadedVdbIndex != this->vdbFileIndex) {
		loadVDB(vdb_files[this->vdbFileIndex]);
		this->loadedVdbIndex = this->vdbFileIndex;
	}

	if (mesh && this->shader)
	{
		// 1. Activar el shader
		this->shader->enable();

		// 2. Llamar a tu función setUniforms pasando los parámetros necesarios
		setUniforms(camera, model, mesh);

		// Lab 4 Task 2: send the first light of the scene (position, colour, intensity)
		if (!Application::instance->light_list.empty())
			Application::instance->light_list[0]->setUniforms(this->shader, model);
		else
			this->shader->setUniform("u_light_intensity", 0.f);

		// 3. Realizar la llamada de dibujo (draw call) de la geometría auxiliar (el cubo)
		mesh->render(GL_TRIANGLES);

		// 4. Desactivar el shader
		this->shader->disable();
	}
}

void VolumeMaterial::renderInMenu()
{
	ImGui::Separator();
	ImGui::Text("Volume Settings");
	ImGui::Text("Material Type: %s", std::string("Volume").c_str());
	ImGui::SliderFloat("Change absortion coefficient",&this->absortion_coefficient, 0.0f, 40.0f); // VDB densities are low (bunny ~0.13), so allow larger values
	const char* task_items[] = {
		"Task 1: Homogeneous Absorption",
		"Task 2: Heterogeneous Absorption",
		"Task 3: Emission-Absorption",
		"Lab 4: Full model (scattering)"
	};

	// ComboBox de ImGui vinculado a la variable 'task'
	if (ImGui::Combo("Task Model", &this->Task, task_items, 4)) {
		// Opcional: código que se ejecute justo al cambiar de opción
	}

	// Opcional: Mostrar controles adicionales solo cuando se seleccione la Task 3 (Emisión)
	if (this->Task == 2 || this->Task == 3) {

		ImGui::ColorEdit3("EmitedLight", (float*)&emitedLight);
	}
	ImGui::SliderFloat("Step Size", &this->stepsize, 0.001f, 0.2f);
	ImGui::SliderFloat("Noise Factor", &this->noisefactor, 0.0f, 10.0f);

	// Lab 4 Task 2: scattering controls (the light itself is edited in its own node)
	if (this->Task == 3) {
		ImGui::SliderFloat("Scattering coefficient", &this->scattering_coefficient, 0.0f, 40.0f);
		ImGui::SliderInt("Light ray-march steps", &this->lightSteps, 1, 64);
		ImGui::SliderFloat("Phase asymmetry g", &this->g, -0.99f, 0.99f); // |g| = 1 would be a delta
	}

	// Lab 4 Task 1: choose where the density comes from (used by the heterogeneous models)
	const char* source_items[] = { "3D noise", "VDB file" };
	ImGui::Combo("Density source", &this->densitySource, source_items, 2);
	if (this->densitySource == 1) {
		ImGui::Combo("VDB file", &this->vdbFileIndex, vdb_names, 2);
		if (!this->texture)
			ImGui::Text("VDB file missing: using no density");
	}

	
}

void WireframeMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	if (this->shader && mesh)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);

		// Enable shader
		this->shader->enable();

		// Upload material specific uniforms
		setUniforms(camera, model);

		// Do the draw call
		mesh->render(GL_TRIANGLES);

		glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

StandardMaterial::StandardMaterial(glm::vec4 color)
{
	this->color = color;
	this->base_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/basic.fs");
	this->normal_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/normal.fs");
	this->shader = this->base_shader;
}

StandardMaterial::~StandardMaterial() { }

void StandardMaterial::setUniforms(Camera* camera, glm::mat4 model)
{
	// Upload node uniforms
	this->shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	this->shader->setUniform("u_camera_position", camera->eye);
	this->shader->setUniform("u_model", model);

	this->shader->setUniform("u_color", this->color);

	if (this->texture) {
		this->shader->setUniform("u_texture", this->texture, 0);
	}
}

void StandardMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	bool first_pass = true;
	if (mesh && this->shader)
	{
		// Enable shader
		this->shader->enable();

		// Multi pass render
		int num_lights = (int)Application::instance->light_list.size();
		for (int nlight = -1; nlight < num_lights; nlight++)
		{
			if (nlight == -1) { nlight++; } // hotfix

			// Upload uniforms
			setUniforms(camera, model);

			// Upload light uniforms
			if (!first_pass) {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				glDepthFunc(GL_LEQUAL);
			}
			this->shader->setUniform("u_ambient_light", Application::instance->ambient_light * (float)first_pass);

			if (num_lights > 0) {
				Light* light = Application::instance->light_list[nlight];
				light->setUniforms(this->shader, model);
			}
			else {
				// Set some uniforms in case there is no light
				this->shader->setUniform("u_light_intensity", 1.f);
				this->shader->setUniform("u_light_shininess", 1.f);
				this->shader->setUniform("u_light_color", glm::vec4(0.f));
			}

			// Do the draw call
			mesh->render(GL_TRIANGLES);
            
			first_pass = false;
		}

		// Disable shader
		this->shader->disable();
	}
}

void StandardMaterial::renderInMenu()
{
	ImGui::Text("Material Type: %s", std::string("Standard").c_str());

	if (ImGui::Checkbox("Show Normals", &this->show_normals)) {
		if (this->show_normals) {
			this->shader = this->normal_shader;
		}
		else {
			this->shader = this->base_shader;
		}
	}

	if (!this->show_normals) ImGui::ColorEdit3("Color", (float*)&this->color);
}
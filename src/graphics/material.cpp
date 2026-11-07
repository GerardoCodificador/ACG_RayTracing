#include "material.h"

#include "application.h"

#include <istream>
#include <fstream>
#include <algorithm>

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
	this->Task = 2;
	this->stepsize = 0.01f;
	this->absortion_coefficient = ac;
	this->color = glm::vec4(1.f);
	this->shader = Shader::Get("res/shaders/basic.vs", "res/shaders/volume.fs");
	this->emitedLight = glm::vec3(0.5f, 0.2f, 0.7f);
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
}

void VolumeMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	if (mesh && this->shader)
	{
		// 1. Activar el shader
		this->shader->enable();

		// 2. Llamar a tu función setUniforms pasando los parámetros necesarios
		setUniforms(camera, model, mesh);

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
	ImGui::SliderFloat("Change absortion coefficient",&this->absortion_coefficient, 0.0f, 5.0f);
	const char* task_items[] = {
		"Task 1: Homogeneous Absorption",
		"Task 2: Heterogeneous Absorption",
		"Task 3: Emission-Absorption"
	};

	// ComboBox de ImGui vinculado a la variable 'task'
	if (ImGui::Combo("Task Model", &this->Task, task_items, 3)) {
		// Opcional: código que se ejecute justo al cambiar de opción
	}

	// Opcional: Mostrar controles adicionales solo cuando se seleccione la Task 3 (Emisión)
	if (this->Task == 2) {

		ImGui::ColorEdit4("EmitedLight", (float*)&emitedLight);
	}
	ImGui::SliderFloat("Step Size", &this->stepsize, 0.001f, 0.2f);
	ImGui::SliderFloat("Noise Factor", &this->noisefactor, 0.0f, 10.0f);

	
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
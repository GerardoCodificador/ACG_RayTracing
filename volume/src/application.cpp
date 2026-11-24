#include "application.h"
#include "graphics/texture.h"

#include <ctime>
#include <filesystem>

bool render_wireframe = false;
glm::vec4 backgroundcolor;
Camera* Application::camera = nullptr;

void Application::init(GLFWwindow* window)
{
    this->instance = this;
    glfwGetFramebufferSize(window, &this->window_width, &this->window_height);

    // OpenGL flags
    glEnable(GL_CULL_FACE); // render both sides of every triangle
    glEnable(GL_DEPTH_TEST); // check the occlusions using the Z buffer

    // Create camera
    this->camera = new Camera();
    this->camera->lookAt(glm::vec3(1.f, 1.5f, 4.f), glm::vec3(0.f, 0.0f, 0.f), glm::vec3(0.f, 1.f, 0.f));
    this->camera->setPerspective(60.f, this->window_width / (float)this->window_height, 0.1f, 500.f); // set the projection to perspective

    this->flag_grid = true;
    this->flag_wireframe = false;

    this->ambient_light = glm::vec4(0.75f, 0.75f, 0.75f, 1.f);
    backgroundcolor=glm::vec4(0.15f, 0.75f, 0.15f, 1.f);
    /* ADD NODES TO THE SCENE */
    SceneNode* example = new SceneNode("Example Node");
    SceneNode* volume = new SceneNode("Volume Node");
    example->mesh = Mesh::Get("res/meshes/sphere.obj");
    volume->mesh = Mesh::Get("res/meshes/cube.obj");
    volume->model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
    example->material = new StandardMaterial();

    volume->material = new VolumeMaterial(5.0f,backgroundcolor);

    this->node_list.push_back(example);
    this->node_list.push_back(volume);

    // Lab 4 Task 2: a point light for the volume to scatter. It goes in the light list (read by
    // the volume material) and in the node list (so it is drawn and can be moved in the GUI)
    Light* light = new Light(glm::vec3(4.0f, 1.8f, 1.0f), 20.f);
    this->light_list.push_back(light);
    this->node_list.push_back(light);
}

void Application::update(float dt)
{
    // Mouse update
    glm::vec2 delta = this->lastMousePosition - this->mousePosition;
    if (this->dragging) {
        this->camera->orbit(-delta.x * dt, delta.y * dt);
    }
    this->lastMousePosition = this->mousePosition;
}

void Application::render()
{
    // Set the clear color (the background color)
    glClearColor(backgroundcolor.x,backgroundcolor.y,backgroundcolor.z,backgroundcolor.w);

    // Clear the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set flags
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    for (unsigned int i = 0; i < this->node_list.size(); i++)
    {
        this->node_list[i]->render(this->camera);

        if (this->flag_wireframe) this->node_list[i]->renderWireframe(this->camera);
    }

    // Draw the floor grid
    if (this->flag_grid) drawGrid();

    // The GUI is drawn after this, so the saved image only contains the scene
    if (this->flag_save_render) {
        saveRender();
        this->flag_save_render = false;
    }
}

// Save the current frame (read back from the framebuffer) to renders/render_<date>_<time>.tga
void Application::saveRender()
{
    // The viewport is the real framebuffer size, which is larger than the window on Retina screens
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    Image image;
    image.fromScreen(viewport[2], viewport[3]);

    std::filesystem::create_directories("renders");
    char filename[64];
    std::time_t now = std::time(nullptr);
    std::strftime(filename, sizeof(filename), "renders/render_%Y%m%d_%H%M%S.tga", std::localtime(&now));

    if (image.saveTGA(filename))
        std::cout << "Render saved to " << filename << std::endl;
}

void Application::renderGUI()
{
    ImGui::Text("Press P to save the render to renders/");

    if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::ColorEdit4("Background Color", (float*)&backgroundcolor);
        ImGui::ColorEdit3("Ambient light", (float*)&this->ambient_light);

        if (ImGui::TreeNode("Camera")) {
            this->camera->renderInMenu();
            ImGui::TreePop();
        }

        unsigned int count = 0;
        std::stringstream ss;
        for (auto& node : this->node_list) {
            ss << count;
            if (ImGui::TreeNode(node->name.c_str())) {
                node->renderInMenu();
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}

void Application::shutdown() { }

// keycodes: https://www.glfw.org/docs/3.3/group__keys.html
void Application::onKeyDown(int key, int scancode)
{
    switch (key) {
    case GLFW_KEY_ESCAPE: // quit
        close = true;
        break;
    case GLFW_KEY_R:
        Shader::ReloadAll();
        break;
    case GLFW_KEY_P: // save the next frame to a file
        flag_save_render = true;
        break;
    }
}

// keycodes: https://www.glfw.org/docs/3.3/group__keys.html
void Application::onKeyUp(int key, int scancode)
{
    switch (key) {
    case GLFW_KEY_T:
        std::cout << "T released" << std::endl;
        break;
    }
}

void Application::onRightMouseDown()
{
    this->dragging = true;
    this->lastMousePosition = this->mousePosition;
}

void Application::onRightMouseUp()
{
    this->dragging = false;
    this->lastMousePosition = this->mousePosition;
}

void Application::onLeftMouseDown()
{
    this->dragging = true;
    this->lastMousePosition = this->mousePosition;
}

void Application::onLeftMouseUp()
{
    this->dragging = false;
    this->lastMousePosition = this->mousePosition;
}

void Application::onMiddleMouseDown() { }

void Application::onMiddleMouseUp() { }

void Application::onMousePosition(double xpos, double ypos) { }

void Application::onScroll(double xOffset, double yOffset)
{
    int min = (int)this->camera->min_fov;
    int max = (int)this->camera->max_fov;

    if (yOffset < 0) {
        this->camera->fov += 4.f;
        if (this->camera->fov > max) {
            this->camera->fov = (float)max;
        }
    }
    else {
        this->camera->fov -= 4.f;
        if (this->camera->fov < min) {
            this->camera->fov = (float)min;
        }
    }
    this->camera->updateProjectionMatrix();
}

void Application::onWindowSize(int width, int height)
{
    glViewport(0, 0, width, height);
    this->camera->setAspectRatio(width / (float)height);
    this->camera->updateProjectionMatrix();
}
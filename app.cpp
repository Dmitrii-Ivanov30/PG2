#include "app.hpp"


App::App() {
    window = nullptr;
}

void App::mouse_clicked_callback(GLFWwindow* window, int button, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (app->currentColor.r == 1.0f) {
            app->currentColor.r = 0.0f;
            app->currentColor.g = 1.0f;
        } else if (app->currentColor.g == 1.0f) {
            app->currentColor.g = 0.0f;
            app->currentColor.b = 1.0f;
        } else if (app->currentColor.b == 1.0f) {
            app->currentColor.b = 0.0f;
            app->currentColor.r = 1.0f;
        }
    }
}

bool App::init() {
    // init GLFW
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // create window
    window = glfwCreateWindow(800, 600, "OpenGL Scene", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);

    // init GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLEW");
    }

    // activate callbacks
    glfwSetWindowUserPointer(window, this);
    glfwSetMouseButtonCallback(window, mouse_clicked_callback);

    // init resources
    try {
        init_assets();
    }
    catch (const std::exception& e) {
        std::cerr << "Asset initialization failed: " << e.what() << std::endl;
        return false;
    }

    return true;
}

void App::init_assets(void) {
    // load shader program
    shader = ShaderProgram("resources/shaders/basic.vert", "resources/shaders/basic.frag");

    // load model
    Model triangle_model("resources/objects/triangle.obj", shader);

    // add to scene
    scene.emplace("triangle", std::move(triangle_model));
}

int App::run() {

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate main shader and set uniforms
        shader.activate();
        shader.setUniform("uniform_Color", currentColor);

        // Draw all models in the scene
        for (auto& [name, model] : scene) {
            model.draw();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    return EXIT_SUCCESS;
}


App::~App() {
    // cleanup models and shaders
    scene.clear();
    shader.clear();

    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
    std::cout << "Application shutdown successfully\n";

}

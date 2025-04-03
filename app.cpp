#include "app.hpp"


App::App() {
    window = nullptr;
	std::cout << "Application initialized\n";
}

void App::loadConfig() {
	// load window configuration from JSON file
    try {
        std::ifstream configFile("app_settings.json");
        if (!configFile.is_open()) {
            throw std::runtime_error("Failed to open config file");
        }

        // parse JSON
		nlohmann::json config = nlohmann::json::parse(configFile);

        
		windowWidth = config["default_resolution"].value("x", 800);
		windowHeight = config["default_resolution"].value("y", 600);
		windowTitle = config.value("appname", "OpenGL Scene");

		// close file
		configFile.close();

		std::cout << "Window configuration loaded successfully:\n";
	}
	catch (const std::exception& e) {
		std::cerr << "Error loading window configurations: " << e.what() 
            << " using default settings" << std::endl;
    }
}

void App::printGLInfo() {
    // basic OpenGL information
    std::cout << "\nOpenGL Context Information:" << std::endl;
    std::cout << "===========================" << std::endl;

    // vendor and renderer information
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    std::cout << "Vendor: \t" << (vendor ? vendor : "<Unknown>") << '\n';

    const char* renderer = (const char*)glGetString(GL_RENDERER);
    std::cout << "Renderer: \t" << (renderer ? renderer : "<Unknown>") << '\n';

    // version information
    const char* gl_version = (const char*)glGetString(GL_VERSION);
    std::cout << "OpenGL Version: \t" << (gl_version ? gl_version : "<Unknown>") << '\n';

    const char* glsl_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    std::cout << "GLSL Version: \t\t" << (glsl_version ? glsl_version : "<Unknown>") << '\n';

    // numeric version verification
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    std::cout << "OpenGL Context Version: \t" << major << "." << minor << '\n';

    if (major < 4 || (major == 4 && minor < 6)) {
        throw std::runtime_error("OpenGL 4.6 context not created!");
    }

    // profile information
    GLint profile_mask;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile_mask);
    std::cout << "Context Profile: \t";

    if (profile_mask & GL_CONTEXT_CORE_PROFILE_BIT) {
        std::cout << "Core Profile";
    }
    else if (profile_mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) {
        std::cout << "Compatibility Profile";
    }
    else {
        std::cout << "<Unknown Profile>";
    }
    std::cout << '\n';

    // context flags
    GLint context_flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
    std::cout << "Context Flags: \t\t";

    if (context_flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
        std::cout << "[Forward Compatible] ";
    if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT)
        std::cout << "[Debug] ";
    if (context_flags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT)
        std::cout << "[Robust Access] ";
    if (context_flags & GL_CONTEXT_FLAG_NO_ERROR_BIT)
        std::cout << "[No Error] ";

    std::cout << "\n===========================\n\n";
}


bool App::init() {
    // request debug context
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	// load window configuration
    loadConfig();

    // init GLFW
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // create window
    window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), nullptr, nullptr);
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

	// print OpenGL information
    std::cout << "\nInitializing OpenGL context...\n";
    printGLInfo();

	// print OpenGL errors
    if (GLEW_ARB_debug_output)
    {
        glDebugMessageCallback(MessageCallback, 0);
        glEnable(GL_DEBUG_OUTPUT);

        //default is asynchronous debug output, use this to simulate glGetError() functionality
        //glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        std::cout << "GL_DEBUG enabled.\n" << std::endl;
    }
    else
        std::cout << "GL_DEBUG NOT SUPPORTED!\n" << std::endl;

    // aktivate Vsync
    glfwSwapInterval(vsync ? 1 : 0);

    // activate callbacks
    glfwSetWindowUserPointer(window, this);
    glfwSetMouseButtonCallback(window, mouse_clicked_callback);
	glfwSetKeyCallback(window, key_callback);

    // init resources
    try {
        initAssets();
		std::cout << "Assets initialized successfully\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Asset initialization failed: " << e.what() << std::endl;
        return false;
    }

    return true;
}


void App::initAssets(void) {
    // load shader program
    shader = ShaderProgram("resources/shaders/basic.vert", "resources/shaders/basic.frag");

    // load model
    Model triangle_model("resources/objects/triangle.obj", shader);

    // add to scene
    scene.emplace("triangle", std::move(triangle_model));
}


int App::run() {
    glEnable(GL_DEPTH_TEST);

    // FPS counting variables
    double lastTime = glfwGetTime();
    int frameCount = 0;

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

        // FPS calculation
        frameCount++;
        const double current_time = glfwGetTime();
        const double elapsed = current_time - lastTime;
		// update window title every second
        if (elapsed >= 1.0) {
            int fps = static_cast<int>(frameCount / elapsed);
			// show title + fps + vsync status
            std::string title = windowTitle + " [FPS: " + std::to_string(fps) + "], VSYNC: " + (vsync ? "ON" : "OFF");
            glfwSetWindowTitle(window, title.c_str());

            frameCount = 0;
            lastTime = current_time;
        }
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

// ----- callbacks ------
void App::mouse_clicked_callback(GLFWwindow* window, int button, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (app->currentColor.r == 1.0f) {
            app->currentColor.r = 0.0f;
            app->currentColor.g = 1.0f;
        }
        else if (app->currentColor.g == 1.0f) {
            app->currentColor.g = 0.0f;
            app->currentColor.b = 1.0f;
        }
        else if (app->currentColor.b == 1.0f) {
            app->currentColor.b = 0.0f;
            app->currentColor.r = 1.0f;
        }
    }
}


void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	std::cout << "Activate key_callback: Key pressed: " << key << std::endl;
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if ((action == GLFW_PRESS) || (action == GLFW_REPEAT))
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_V:
            if (app->vsync) {
                glfwSwapInterval(0);          // Set V-Sync OFF.
				app->vsync = false;
            }
            else {
                glfwSwapInterval(1);        // Set V-Sync ON.
				app->vsync = true;
            }
            break;
        default:
            break;
        }
    }
}
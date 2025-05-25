#include "app.hpp"


App::App() : window(nullptr), fov(60.0f), vsync(true), currentColor(1.0f, 0.0f, 0.0f, 1.0f) {
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
        fov = config.value("fov", 60.0f);
        AA = config["AA"].value("enabled", false);
        AASamples = config["AA"].value("samples", 0);
        // close file
        configFile.close();

        std::cout << "Window configuration loaded successfully:\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading window configurations: " << e.what()
            << " using default settings" << std::endl;
        windowWidth = 800;
        windowHeight = 600;
        fov = 60.0f;
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
	// initialize GLFW window hints
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    // assume ALL objects are non-transparent 
    glEnable(GL_CULL_FACE);

    // request debug context
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    // load window configuration
    loadConfig();

    // init GLFW
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    // request MSAA
    if (AA) glfwWindowHint(GLFW_SAMPLES, AASamples);
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
    // enable antialiasing
    if (AA) glEnable(GL_MULTISAMPLE);

    // initial view matrix
    updateProjection();
    //viewMatrix = glm::lookAt(
    //    glm::vec3(0.0f, 0.0f, 3.0f), // Camera position
    //    glm::vec3(0.0f, 0.0f, 0.0f), // Look at
    //    glm::vec3(0.0f, 1.0f, 0.0f)  // Up vector
    //);
    camera.position = glm::vec3(0.0f, 0.0f, 3.0f);
    viewMatrix = camera.GetViewMatrix();

    // print OpenGL information
    std::cout << "\nInitializing OpenGL context...\n";
    printGLInfo();

    // print OpenGL errors
    if (GLEW_ARB_debug_output) {
        glDebugMessageCallback(MessageCallback, 0);
        glEnable(GL_DEBUG_OUTPUT);
        std::cout << "GL_DEBUG enabled.\n" << std::endl;
    }
    else {
        std::cout << "GL_DEBUG NOT SUPPORTED!\n" << std::endl;
    }

    // activate Vsync
    glfwSwapInterval(vsync ? 1 : 0);

    // activate callbacks
    glfwSetWindowUserPointer(window, this);
    glfwSetMouseButtonCallback(window, mouse_clicked_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback); // mouse movement

    // init resources
    try {
        initAssets();
        std::cout << "Assets initialized successfully\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Asset initialization failed: " << e.what() << std::endl;
        return false;
    }
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glDepthFunc(GL_LEQUAL);
    return true;
}

void App::initAssets(void) {
    /*
     * Terrain init
     */
    bool isTransparent = false;
    shader = ShaderProgram("resources/shaders/tex.vert", "resources/shaders/tex.frag");
    Model terrainModel(shader);
    //GLuint texture_terrain = textureInit("resources/textures/box_rgb888.png", isTransparent);
    //GLuint texture_terrain = textureInit("resources/textures/green_text.jpg", isTransparent);
    GLuint texture_terrain = textureInit("resources/textures/tex_256.png", isTransparent);

    terrainModel.transparent = isTransparent;
    for (auto& mesh : terrainModel.meshes) {
        mesh.texture_id = texture_terrain;
    }
    scene.emplace("terrain", std::move(terrainModel));

    /*
     * Triangle init
     */
    isTransparent = true;
    // load shader program
    shader = ShaderProgram("resources/shaders/tex_1.vert", "resources/shaders/tex_1.frag");

    // load model
    Model triangleModel("resources/objects/triangle.obj", shader);
    triangleModel.origin = glm::vec3(0.0f, 0.0f, 0.0f);  // center the model

    // load texture
    GLuint texture = textureInit("resources/textures/transparent4.png", isTransparent);
    triangleModel.transparent = isTransparent;
    // assign all textures to all meshes
    for (auto& mesh : triangleModel.meshes) {
        mesh.texture_id = texture;
    }

    // add to scene
    //scene.emplace("triangle", std::move(triangleModel));

    // initialize lights
    initLights();

}

void App::updateProjection() {
    float aspect = static_cast<float>(windowWidth) / windowHeight;
    projectionMatrix = glm::perspective(
        glm::radians(fov), aspect, 0.1f, 100.0f
    );
}

GLuint App::textureInit(const std::filesystem::path& file_name, bool& isTransparent)
{
    cv::Mat image = cv::imread(file_name.string(), cv::IMREAD_UNCHANGED);  // Read with (potential) Alpha
    if (image.empty()) {
        throw std::runtime_error("No texture in file: " + file_name.string());
    }

    // or print warning, and generate synthetic image with checkerboard pattern 
    // using OpenCV and use as a texture replacement

    GLuint texture = gen_tex(image, isTransparent);

    return texture;
}

GLuint App::gen_tex(cv::Mat& image, bool& isTransparent)
{
    GLuint ID = 0;
    if (image.empty())
        throw std::runtime_error("Image empty?\n");


    // Generates an OpenGL texture object
    glCreateTextures(GL_TEXTURE_2D, 1, &ID);

    switch (image.channels()) {
    case 3:
        // Create and clear space for data - immutable format
        glTextureStorage2D(ID, 1, GL_RGB8, image.cols, image.rows);
        // Assigns the image to the OpenGL Texture object
        glTextureSubImage2D(ID, 0, 0, 0, image.cols, image.rows, GL_BGR, GL_UNSIGNED_BYTE, image.data);
        break;
    case 4:
        for (int y = 0; y < image.rows && !isTransparent; ++y) {
            for (int x = 0; x < image.cols; ++x) {
                cv::Vec4b pixel = image.at<cv::Vec4b>(y, x);
                if (pixel[3] < 255) { // pixel[3] is alpha
                    isTransparent = true;
                    break;
                }
            }
        }
        glTextureStorage2D(ID, 1, GL_RGBA8, image.cols, image.rows);
        glTextureSubImage2D(ID, 0, 0, 0, image.cols, image.rows, GL_BGRA, GL_UNSIGNED_BYTE, image.data);
        break;
    default:
        throw std::runtime_error("unsupported channel cnt. in texture:" + std::to_string(image.channels()));
    }

    // MIPMAP filtering + automatic MIPMAP generation - nicest, needs more memory. Notice: MIPMAP is only for image minifying.
    glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // bilinear magnifying
    glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // trilinear minifying
    glGenerateTextureMipmap(ID);  //Generate mipmaps now.

    // Configures the way the texture repeats
    glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return ID;
}

//void App::initLights() {
//    // init point lights from the file
//    std::filesystem::path point_lights_path = "resources/lights/point_lights.lights";
//    std::ifstream file_point_light(point_lights_path);
//
//    if (!file_point_light.is_open()) {
//        std::cout << "Could not open point light file: " << point_lights_path << std::endl;
//    }
//
//    std::string line;
//    while (std::getline(file_point_light, line)) {
//        if (line.empty() || line[0] == '#')
//            continue;
//        std::istringstream ss(line);
//        float x, y, z, r, g, b;
//
//        if (!(ss >> x >> y >> z >> r >> g >> b)) {
//            std::cerr << "Invalid point light entry: " << line << std::endl;
//            continue; // or throw
//        }
//        lights.initPointLight(glm::vec3(x, y, z), glm::vec3(r, g, b));
//    }
//    file_point_light.close();
//
//    // init spot lights from the file
//    std::filesystem::path spot_lights_path = "resources/lights/point_lights.lights";
//    std::ifstream file_spot_light(spot_lights_path);
//
//    if (!file_spot_light.is_open()) {
//        std::cout << "Could not open spot light file: " << point_lights_path << std::endl;
//    }
//
//    while (std::getline(file_spot_light, line)) {
//        if (line.empty() || line[0] == '#')
//            continue;
//        std::istringstream ss(line);
//        float posX, posY, posZ, dirX, dirY, dirZ;
//
//        if (!(ss >> posX >> posY >> posZ >> dirX >> dirY >> dirZ)) {
//            std::cerr << "Invalid spot light entry: " << line << std::endl;
//        }
//        lights.initSpotLight(glm::vec3(posX, posY, posZ),
//            glm::vec3(dirX, dirY, dirZ));
//    }
//    file_spot_light.close();
//
//    lights.initCameraLight(camera.position, camera.front);
//    lights.initDirectionalLight();
//}
void App::initLights() {}

int App::run() {
    // Enable back-face culling to improve performance by not rendering polygons facing away from the camera
    // glCullFace(GL_BACK);
    // glEnable(GL_CULL_FACE);

    // Initialize camera settings
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // capture mouse
    glfwGetCursorPos(window, &cursorLastX, &cursorLastY);        // get initial position

    glViewport(0, 0, windowWidth, windowHeight);

    // time variables
    double lastTime = glfwGetTime();
    double lastFrameTime = glfwGetTime();
    double deltaTime = 0.0;
    int frameCount = 0;

    // Print number of lights before drawing
    std::cout << "numPointLights = " << lights.pointLights.size()
        << ", numSpotLights = " << lights.spotLights.size() << std::endl;

    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        double currentFrameTime = glfwGetTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // Process camera movement
        glm::vec3 moveOffset = camera.ProcessInput(window, deltaTime);
        camera.position += moveOffset;

        // Update spotlight position/direction to follow camera
        lights.cameraLight.position = camera.position;
        lights.cameraLight.direction = camera.front;

        // Update view matrix from camera
        viewMatrix = camera.GetViewMatrix();

        // Clear buffers
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate main shader and set uniforms
        // shader.activate();
        // shader.setUniform("uP_m", projectionMatrix);
        // shader.setUniform("uV_m", viewMatrix); // Updated every frame

        std::vector<Model*> transparent;    // temporary, vector of pointers to transparent objects
        transparent.reserve(scene.size());  // reserve size for all objects to avoid reallocation


        // Draw all models in the scene
        for (auto & [name, model] : scene) {
            if (!model.transparent)
                model.draw(projectionMatrix, viewMatrix, lights);
            else
                transparent.emplace_back(&model); // save pointer for painters algorithm
        }
        // SECOND PART - draw only transparent - painter's algorithm (sort by distance from camera, from far to near)
        std::sort(transparent.begin(), transparent.end(), [&](Model const * a, Model const * b) {
            glm::vec3 translation_a = glm::vec3(a->modelMatrix[3]);  // get 3 values from last column of model matrix = translation
            glm::vec3 translation_b = glm::vec3(b->modelMatrix[3]);  // dtto for model B
            return glm::distance(camera.position, translation_a) < glm::distance(camera.position, translation_b); // sort by distance from camera
            });
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        for (auto p : transparent) {
            p->draw(projectionMatrix, viewMatrix, lights);
        }
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
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

        glfwSwapBuffers(window);  // Update window content
        glfwPollEvents();         // Process pending events
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
    //std::cout << "Activate key_callback: Key pressed: " << key << std::endl;
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if ((action == GLFW_PRESS) || (action == GLFW_REPEAT)) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_V:
            app->vsync = !app->vsync;
            glfwSwapInterval(app->vsync ? 1 : 0);
            break;
        default:
            break;
        }
    }
}

void App::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    //std::cout << "Activate cursor_position_callback." << std::endl;
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    if (app->firstMouse) {
        app->cursorLastX = xpos;
        app->cursorLastY = ypos;
        app->firstMouse = false;
    }

    // calculate offset with inverted Y axis (screen Y goes down, 3D Y goes up)
    float xoffset = xpos - app->cursorLastX;
    float yoffset = app->cursorLastY - ypos; // reversed since y-coordinates go bottom to top

    app->cursorLastX = xpos;
    app->cursorLastY = ypos;

    app->camera.ProcessMouseMovement(xoffset, yoffset);
}
// -----------------------------------------
// icp.cpp 
// author: JJ

#include <iostream>
#include <opencv2/opencv.hpp>

// C++
// include at the beginning, in any order
#include <iostream>

// OpenGL Extension Wrangler: allow all multiplatform GL functions
#include <GL/glew.h>
// WGLEW = Windows GL Extension Wrangler (change for different platform)
// platform specific functions (in this case Windows)
#include <GL/glxew.h>  // GLX extension support on Linux

// GLFW toolkit
// Uses GL calls to open GL context, i.e. GLEW must be first.
#include <GLFW/glfw3.h>

// OpenGL math
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "assets.hpp"
#include "app.hpp"
//---------------------------------------------------------------------

App::App() {
    window = nullptr;
}

void App::mouse_clicked_callback(GLFWwindow* window, int button, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (app->r == 1.0f) {
            app->r = 0.0f;
            app->g = 1.0f;
        } else if (app->g == 1.0f) {
            app->g = 0.0f;
            app->b = 1.0f;
        } else if (app->b == 1.0f) {
            app->b = 0.0f;
            app->r = 1.0f;
        }
    }
}

bool App::init() {
    // init glfw
    // https://www.glfw.org/documentation.html
    glfwInit();

    // open window (GL canvas) with no special properties
    // https://www.glfw.org/docs/latest/quick.html#quick_create_window
    window = glfwCreateWindow(800, 600, "OpenGL context", NULL, NULL);
    glfwMakeContextCurrent(window);

    // init glew
    // http://glew.sourceforge.net/basic.html
    glewInit();
    glxewInit();
    if (!GLEW_ARB_direct_state_access)
        throw std::runtime_error("No DSA :-(");

    init_assets();

    glfwSetWindowUserPointer(window, this);
    glfwSetMouseButtonCallback(window, mouse_clicked_callback);

    return true;
}

void App::init_assets(void) {
    //
    // Initialize pipeline: compile, link and use shaders
    //

    //SHADERS - define & compile & link
    const char* vertex_shader =
        "#version 460 core\n"
        "in vec3 attribute_Position;"
        "void main() {"
        "  gl_Position = vec4(attribute_Position, 1.0);"
        "}";

    const char* fragment_shader =
        "#version 460 core\n"
        "uniform vec4 uniform_Color;"
        "out vec4 FragColor;"
        "void main() {"
        "  FragColor = uniform_Color;"
        "}";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    shader_prog_ID = glCreateProgram();
    glAttachShader(shader_prog_ID, fs);
    glAttachShader(shader_prog_ID, vs);
    glLinkProgram(shader_prog_ID);

    //now we can delete shader parts (they can be reused, if you have more shaders)
    //the final shader program already linked and stored separately
    glDetachShader(shader_prog_ID, fs);
    glDetachShader(shader_prog_ID, vs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    //
    // Create and load data into GPU using OpenGL DSA (Direct State Access)
    //

    // Create VAO + data description (just envelope, or container...)
    glCreateVertexArrays(1, &VAO_ID);

    GLint position_attrib_location = glGetAttribLocation(shader_prog_ID, "attribute_Position");

    glEnableVertexArrayAttrib(VAO_ID, position_attrib_location);
    glVertexArrayAttribFormat(VAO_ID, position_attrib_location, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, position));
    glVertexArrayAttribBinding(VAO_ID, position_attrib_location, 0); // (GLuint vaobj, GLuint attribindex, GLuint bindingindex)

    // Create and fill data
    glCreateBuffers(1, &VBO_ID);
    glNamedBufferData(VBO_ID, triangle_vertices.size()*sizeof(vertex), triangle_vertices.data(), GL_STATIC_DRAW);

    // Connect together
    glVertexArrayVertexBuffer(VAO_ID, 0, VBO_ID, 0, sizeof(vertex)); // (GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
}

int App::run() {

    // Activate shader program. There is only one program, so activation can be out of the loop.
    // In more realistic scenarios, you will activate different shaders for different 3D objects.
    glUseProgram(shader_prog_ID);

    // Get uniform location in GPU program. This will not change, so it can be moved out of the game loop.
    GLint uniform_color_location = glGetUniformLocation(shader_prog_ID, "uniform_Color");
    if (uniform_color_location == -1) {
        std::cerr << "Uniform location is not found in active shader program. Did you forget to activate it?\n";
    }

    while (!glfwWindowShouldClose(window)) {
        // clear canvas
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //set uniform parameter for shader
        // (try to change the color in key callback)
        glUniform4f(uniform_color_location, r, g, b, a);

        //bind 3d object data
        glBindVertexArray(VAO_ID);

        // draw all VAO data
        glDrawArrays(GL_TRIANGLES, 0, triangle_vertices.size());

        // poll events, call callbacks, flip back<->front buffer
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    return 1;
}


App::~App() {

    //new stuff: cleanup GL data
    glDeleteProgram(shader_prog_ID);
    glDeleteBuffers(1, &VBO_ID);
    glDeleteVertexArrays(1, &VAO_ID);

    if (window)
        glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);

}

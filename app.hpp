#pragma once  

// OpenCV (does not depend on GL)  
//#include <opencv2\opencv.hpp>  

// include anywhere, in any order  
#include <iostream>  
#include <fstream>
#include <chrono>  
#include <stack>  
#include <random>  
#include <vector>  
#include <unordered_map>  
#include <nlohmann/json.hpp>  

// OpenGL Extension Wrangler: allow all multiplatform GL functions  
#include <GL/glew.h>  
// WGLEW = Windows GL Extension Wrangler (change for different platform)  
// platform specific functions (in this case Windows)  
#include <GL/wglew.h>  
#include <GL/gl.h>  

// GLFW toolkit  
// Uses GL calls to open GL context, i.e. GLEW __MUST__ be first.  
#include <GLFW/glfw3.h>  

// OpenGL math (and other additional GL libraries, at the end)  
#include <glm/glm.hpp>  
#include <glm/gtc/type_ptr.hpp>  

// User includes  
#include "assets.hpp"  
#include "ShaderProgram.hpp"  
#include "Model.hpp"  
#include "Mesh.hpp"

// callbacks
#include "gl_err_callback.h"

class App {  
public:  
    GLFWwindow * window;  
    App();  

    bool init();  
    int run();  
    void initAssets();  

    static void mouse_clicked_callback(GLFWwindow *window, int button, int action, int mods);  

    ~App();  

protected:  
    // all objects of the scene addressable by name  
    std::unordered_map<std::string, Model> scene;  
    ShaderProgram shader;  

private:  
	// default window settings
	int windowWidth{ 800 };
	int windowHeight{ 600 };
	std::string windowTitle{ "OpenGL Scene" }; 
    // color uniform state  
    glm::vec4 currentColor{ 1.0f, 0.0f, 0.0f, 1.0f };  // RGBA format  

	void loadConfig();
	void printGLInfo();
};

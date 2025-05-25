#pragma once
#include <glm/glm.hpp>
#include <vector>


struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    static DirectionalLight createDefault();
};

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant;
    float linear;
    float quadratic;

    static PointLight createDefault(const glm::vec3& position, const glm::vec3& color);
};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant;
    float linear;
    float quadratic;

    static SpotLight createDefault(const glm::vec3& pos, const glm::vec3& dir);
};

struct Lights {
    DirectionalLight sun;
    std::vector<SpotLight> spotLights;
    SpotLight cameraLight;
    std::vector<PointLight> pointLights;

    void initDirectionalLight();
    void initPointLight(const glm::vec3& position, const glm::vec3& color);
    void initSpotLight(const glm::vec3& pos, const glm::vec3& dir);
    void initCameraLight(const glm::vec3& pos, const glm::vec3& dir);
};
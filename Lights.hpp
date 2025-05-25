#pragma once
#include <glm/glm.hpp>
#include <array>

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

namespace Lights {
    void initDirectionalLight(DirectionalLight& light);
    void initPointLights(std::array<PointLight, 3>& pointLights);
    void initSpotLight(SpotLight& spotLight, const glm::vec3& pos, const glm::vec3& dir);
}
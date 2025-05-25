#include "Lights.hpp"

DirectionalLight DirectionalLight::createDefault() {
    return DirectionalLight{
        glm::vec3(-0.2f, -1.0f, -0.3f),  // direction
        glm::vec3(0.2f),                 // ambient
        glm::vec3(0.5f),                 // diffuse
        glm::vec3(1.0f)                  // specular
    };
}

PointLight PointLight::createDefault(const glm::vec3& position, const glm::vec3& color) {
    return PointLight{
        position,                   // position
        color * 0.1f,              // ambient
        color * 0.8f,              // diffuse
        glm::vec3(1.0f),           // specular
        1.0f,                      // constant
        0.09f,                     // linear
        0.032f                     // quadratic
    };
}

SpotLight SpotLight::createDefault(const glm::vec3& pos, const glm::vec3& dir) {
    return SpotLight{
        pos,                                    // position
        dir,                                    // direction
        glm::cos(glm::radians(12.5f)),         // cutOff
        glm::cos(glm::radians(17.5f)),        // outerCutOff
        glm::vec3(0.1f),                      // ambient
        glm::vec3(0.8f),                      // diffuse
        glm::vec3(1.0f),                      // specular
        1.0f,                                 // constant
        0.09f,                                // linear
        0.032f                                // quadratic
    };
}

void Lights::initDirectionalLight() {
    sun = DirectionalLight::createDefault();
}

void Lights::initPointLight(const glm::vec3& position, const glm::vec3& color) {
    pointLights.emplace_back(PointLight::createDefault(position, color));
}

void Lights::initSpotLight(const glm::vec3& pos, const glm::vec3& dir) {
    spotLights.emplace_back(SpotLight::createDefault(pos, dir));
}

void Lights::initCameraLight(const glm::vec3& pos, const glm::vec3& dir) {
    cameraLight = SpotLight::createDefault(pos, dir);
}
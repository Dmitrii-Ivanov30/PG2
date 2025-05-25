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

namespace Lights {
    void initDirectionalLight(DirectionalLight& light) {
        light = DirectionalLight::createDefault();
    }

    void initPointLights(std::array<PointLight, 3>& pointLights) {
        pointLights[0] = PointLight::createDefault(
            glm::vec3(2.0f, 0.5f, 2.0f),
            glm::vec3(1.0f, 1.0f, 0.0f) // Yellow
        );
        pointLights[1] = PointLight::createDefault(
            glm::vec3(-2.0f, 1.0f, -1.0f),
            glm::vec3(1.0f, 0.0f, 0.0f) // Red
        );
        pointLights[2] = PointLight::createDefault(
            glm::vec3(0.0f, 1.0f, -3.0f),
            glm::vec3(0.0f, 1.0f, 0.0f) // Green
        );
    }

    void initSpotLight(SpotLight& spotLight, const glm::vec3& pos, const glm::vec3& dir) {
        spotLight = SpotLight::createDefault(pos, dir);
    }
}
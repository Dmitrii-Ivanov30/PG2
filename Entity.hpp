#pragma once
#include <glm/glm.hpp>
#include <functional>
#include <vector>
#include "Model.hpp"

class Entity {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;

    float yaw = -90.0f, pitch = 0.0f;
    float movementSpeed = 10.0f;
    float drag = 0.1f;
    float gravity = -9.81f;
    bool isGrounded = true;

    Model* model; // optional visual

    using Behavior = std::function<void(Entity&, float)>;
    std::vector<Behavior> behaviors;

    Entity(glm::vec3 pos, Model* mdl = nullptr)
        : position(pos), velocity(0.0f), acceleration(0.0f), model(mdl) {
    }

    void update(float dt, float groundHeight) {
        // Gravity
        if (!isGrounded) acceleration.y += gravity;

        for (auto& b : behaviors) b(*this, dt);

        // Physics
        velocity += acceleration * dt;
        velocity.x *= pow(drag, dt);
        velocity.z *= pow(drag, dt);

        position += velocity * dt;
        if (position.y <= groundHeight) {
            position.y = groundHeight;
            velocity.y = 0;
            isGrounded = true;
        }
        else {
            isGrounded = false;
        }
        acceleration = glm::vec3(0.0f);
    }

    void applyForce(const glm::vec3& force) { acceleration += force; }

    void jump(float strength) {
        if (isGrounded) { velocity.y = strength; isGrounded = false; }
    }
};

#pragma once
#include "Entity.hpp"
#include <glm/glm.hpp>
#include <cmath>
#include <functional>

namespace Behaviors {
    using Behavior = Entity::Behavior;

    // Walk in a circle
    inline Behavior WalkInCircle(glm::vec3 center, float radius, float speed = 1.0f) {
        float angle = 0.0f;
        return [=](Entity& self, float dt) mutable {
            angle += speed * dt;
            glm::vec3 target = center + glm::vec3(cos(angle) * radius, 0, sin(angle) * radius);
            glm::vec3 dir = glm::normalize(target - self.position);
            self.applyForce(dir * self.movementSpeed);
            };
    }

    // Bob up and down
    inline Behavior Bob(float amplitude = 0.5f, float speed = 1.0f) {
		std::cout << "Bob behavior initialized with amplitude: " << amplitude << " and speed: " << speed << std::endl;
        float baseY = 0.0f;
        bool first = true;
        return [=](Entity& self, float dt) mutable {
            if (first) { baseY = self.position.y; first = false; }
            self.position.y = baseY + sin(glfwGetTime() * speed) * amplitude;
            };
    }

}
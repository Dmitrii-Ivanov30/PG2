#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <vector>
#include "ShaderProgram.hpp"

namespace Particles {
    struct Particle {
        glm::vec3 position, velocity;
        float life = 1.0f;
        bool active = false;
    };

	constexpr int MAX_PARTICLES = 1000;                // Maximum number of particles in the pool
	inline std::vector<Particle> pool(MAX_PARTICLES);  // Particle pool

	// Update all active particles
    inline void update(float dt) {
        for (auto& p : pool) {
            if (!p.active) continue;
            p.position += p.velocity * dt;
            p.life -= dt;
            if (p.life <= 0.0f) p.active = false;
        }
    }

	// Spawn new particles at a given origin
    inline void spawn(const glm::vec3& origin, int count = 10) {
        int spawned = 0;
        for (auto& p : pool) {
            if (!p.active) {
                p.position = origin;
                p.velocity = glm::sphericalRand(5.0f);
                p.life = 0.5f + float(rand()) / RAND_MAX;
                p.active = true;
                if (++spawned >= count) break;
            }
        }
    }

    // Draw all active particles as GL_POINTS
    // shader must have a vec4 uniform "color" and a mat4 uniform "uMVP"
    inline void drawParticles(const glm::mat4& projection, const glm::mat4& view, ShaderProgram& shader) {
        std::vector<glm::vec3> points;

		// Collect all active particle positions
        for (const auto& p : pool) {
            if (p.active) {
                points.push_back(p.position);
            }
        }
		// if no active particles, skip rendering
        if (points.empty()) return;

		// Create VAO and VBO for rendering particles
        GLuint VAO = 0, VBO = 0;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

		// Bind VAO and VBO, upload particle positions
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

		// Activate shader and set uniforms
        shader.activate();
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 mvp = projection * view * model;
        shader.setUniform("uMVP", mvp);
        shader.setUniform("color", glm::vec4(1, 0.7f, 0.2f, 1)); // orange sparks

		// Draw particles as points
        glEnable(GL_PROGRAM_POINT_SIZE);
        glPointSize(5.0f);
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(points.size()));

		// Cleanup
        glBindVertexArray(0);
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

}
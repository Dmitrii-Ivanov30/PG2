#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <glm/glm.hpp> 
#include <glm/ext.hpp>

#include "assets.hpp"
#include "ShaderProgram.hpp"

class Mesh {
public:
    // mesh data
    glm::vec3 origin{};
    glm::vec3 orientation{};

    GLuint texture_id{ 0 }; // texture id=0  means no texture
    GLenum primitive_type = GL_POINT;
    ShaderProgram shader;

    // mesh material
    glm::vec4 ambient_material{ 1.0f }; //white, non-transparent 
    glm::vec4 diffuse_material{ 1.0f }; //white, non-transparent 
    glm::vec4 specular_material{ 1.0f }; //white, non-transparent
    float reflectivity{ 1.0f };

    // vertex data
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    // indirect (indexed) draw 
    Mesh(GLenum primitive_type, ShaderProgram shader, std::vector<Vertex> const& vertices, std::vector<GLuint> const& indices, glm::vec3 const& origin, glm::vec3 const& orientation, GLuint const texture_id = 0) :
        primitive_type(primitive_type),
        shader(shader),
        vertices(vertices),
        indices(indices),
        origin(origin),
        orientation(orientation),
        texture_id(texture_id)
    {
        // create and initialize VAO, VBO, EBO and set vertex attribute pointers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        // bind VAO
        glBindVertexArray(VAO);

        // bind and fill VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        // bind and fill EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        // vertex attribute pointers
        // layout location 0: position (vec3)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

        // layout location 1: texcoord (vec2)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));

        // layout location 2: normal (vec3)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

        // unbind VAO (best practice)
        glBindVertexArray(0);
    };


    void draw(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model) {
        shader.activate();

        // Set texture if available
        if (texture_id != 0) {
            glBindTextureUnit(0, texture_id);
            glUniform1i(glGetUniformLocation(shader.getID(), "tex0"), 0);
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        // set transformation matrices
        glm::mat4 mvp = projection * view * model;
        GLint uProj = glGetUniformLocation(shader.getID(), "uP_m");
        GLint uView = glGetUniformLocation(shader.getID(), "uV_m");
        GLint uModel = glGetUniformLocation(shader.getID(), "uM_m");

        if (uProj != -1) glUniformMatrix4fv(uProj, 1, GL_FALSE, &projection[0][0]);
        if (uView != -1) glUniformMatrix4fv(uView, 1, GL_FALSE, &view[0][0]);
        if (uModel != -1) glUniformMatrix4fv(uModel, 1, GL_FALSE, &model[0][0]);

        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(primitive_type, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }


    void clear(void) {
		// clear texture
        if (texture_id != 0) {
            glDeleteTextures(1, &texture_id);
            texture_id = 0;
        }

        primitive_type = GL_POINT;
        // clear rest of the member variables to safe default
        vertices.clear();
        indices.clear();
        origin = glm::vec3(0.0f);
        orientation = glm::vec3(0.0f);

        // delete all allocations 
        if (VBO) { glDeleteBuffers(1, &VBO); VBO = 0; }
        if (EBO) { glDeleteBuffers(1, &EBO); EBO = 0; }
        if (VAO) { glDeleteVertexArrays(1, &VAO); VAO = 0; }
    };

private:
    // OpenGL buffer IDs
    // ID = 0 is reserved (i.e. uninitalized)
    unsigned int VAO{ 0 }, VBO{ 0 }, EBO{ 0 };
};



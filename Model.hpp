#pragma once

#include <filesystem>
#include <string>
#include <vector> 
#include <glm/glm.hpp> 

#include "assets.hpp"
#include "Mesh.hpp"
#include "ShaderProgram.hpp"
#include "OBJloader.hpp"


class Model {
public:
    std::vector<Mesh> meshes;
    std::string name;
    glm::vec3 origin{};
    glm::vec3 orientation{};
    ShaderProgram shader;
    
    Model(const std::filesystem::path & filename, ShaderProgram shader) : shader(shader) {
        // load mesh (all meshes) of the model, (in the future: load material of each mesh, load textures...)
        // call LoadOBJFile, LoadMTLFile (if exist), process data, create mesh and set its properties
        //    notice: you can load multiple meshes and place them to proper positions, 
        //            multiple textures (with reusing) etc. to construct single complicated Model  

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;

        if (!loadOBJ(filename.string().c_str(), positions, uvs, normals)) {
            std::cerr << "Failed to load model: " << filename << std::endl;
            return;
        }

        // build vertex list
        std::vector<Vertex> vertex_data;
        size_t count = std::min({ positions.size(), uvs.size(), normals.size() });
        vertex_data.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            Vertex v;
            v.position = positions[i];
            v.texcoord = uvs[i];
            v.normal = normals[i];
            vertex_data.push_back(v);
        }

        // build index list (simple case: sequential indices)
        std::vector<GLuint> indices(vertex_data.size());
        for (GLuint i = 0; i < indices.size(); ++i) {
            indices[i] = i;
        }

        // create Mesh and store it
        meshes.emplace_back(GL_TRIANGLES, shader, vertex_data, indices, origin, orientation);

        // set model name based on the filename stem
        name = filename.stem().string();
    }

    // update position etc. based on running time
    void update(const float delta_t) {
        // origin += glm::vec3(3,0,0) * delta_t; // s = s0 + v*dt
    }
    
    void draw(glm::vec3 const & offset = glm::vec3(0.0), glm::vec3 const & rotation = glm::vec3(0.0f)) {
        // call draw() on mesh (all meshes)
        for (auto& mesh : meshes) {
            mesh.draw(origin+offset, orientation+rotation);
        }
    }
};


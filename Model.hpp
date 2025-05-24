#pragma once

#include <filesystem>
#include <string>
#include <vector> 
#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>

#include "assets.hpp"
#include "Mesh.hpp"
#include "ShaderProgram.hpp"
#include "OBJloader.hpp"
#include "HeightMap.h"


class Model {
public:
    std::vector<Mesh> meshes;
    std::string name;
    glm::vec3 origin{};
    glm::vec3 orientation{};
    glm::vec3 scale{ 1.0f };
    ShaderProgram shader;
    bool transparent {false};

    glm::mat4 modelMatrix{ 1.0f };  // model matrix for transformations

    // constructor: load model from file
    Model(const std::filesystem::path& filename, ShaderProgram shader) : shader(shader) {
        loadModel(filename);
    }
    Model(ShaderProgram shader) : shader(shader) {
        loadTerrainModel();
        origin = glm::vec3(0.0f, 0.0f, 0.0f);
    };

    // update position etc. based on running time
    void update(const float delta_t) {
        origin += glm::vec3(3,0,0) * delta_t; // s = s0 + v*dt
    }

    void draw(const glm::mat4& projection, const glm::mat4& view) {
        // transformation order: trans -> rotate -> scale
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, origin);
        model = glm::rotate(model, orientation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, orientation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);

        for (auto& mesh : meshes) {
            mesh.draw(projection, view, model);
        }
    }

private:
#include <tuple>
    void loadModel(const std::filesystem::path& path) {
        // load mesh (all meshes) of the model, (in the future: load material of each mesh, load textures...)
        // call LoadOBJFile, LoadMTLFile (if exist), process data, create mesh and set its properties
        //    notice: you can load multiple meshes and place them to proper positions, 
        //            multiple textures (with reusing) etc. to construct single complicated Model  

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;

        if (!loadOBJ(path.string().c_str(), positions, uvs, normals)) {
            std::cerr << "Failed to load model: " << path << std::endl;
            return;
        }

        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        // build vertex data and indices
        for (size_t i = 0; i < positions.size(); ++i) {
            Vertex v;
            v.position = positions[i];
            if (i < uvs.size()) v.texcoord = uvs[i];
            if (i < normals.size()) v.normal = normals[i];
            vertices.push_back(v);
            indices.push_back(static_cast<GLuint>(i));


            // create Mesh and store it
            meshes.emplace_back(GL_TRIANGLES, shader, vertices, indices, origin, orientation);

            // set model name based on the filename stem
            name = path.stem().string();
        }

        std::cout << "Loaded model: " << path << "\n"
            << "Vertices: " << vertices.size() << "\n"
            << "Indices: " << indices.size() << "\n"
            << "Meshes: " << meshes.size() << std::endl;
    }
    void loadTerrainModel() {
        cv::Mat terrain = cv::imread("resources/textures/heights.png", cv::IMREAD_GRAYSCALE);
        if (terrain.empty()) {
            throw std::runtime_error("No heightmap in file: resources/textures/heights.png");
        }
        HeightMap map{};
        auto [vertices, indices] = map.GenHeightMap(terrain, 50);
        Mesh mapMesh(GL_TRIANGLES, shader, vertices, indices, origin, orientation);
        meshes.emplace_back(mapMesh);
        name = "Terrain";
        std::cout << "Loaded heightmap: resources/textures/heights.png" << std::endl;
    }
};

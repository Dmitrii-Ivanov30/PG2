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
    glm::vec3 AABBMin{FLT_MAX};
    glm::vec3 AABBMax{-FLT_MAX};

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

    void draw(const glm::mat4& projection, const glm::mat4& view, const Lights& lights, const glm::vec3* position = nullptr) {
        modelMatrix = glm::mat4(1.0f); // identity matrix
        if (position == nullptr) {
          position = &origin; // use model's origin if no position provided, else the position from the argument
        }
        modelMatrix = glm::translate(modelMatrix, *position);
        modelMatrix = glm::rotate(modelMatrix, orientation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, orientation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = glm::scale(modelMatrix, scale);

        // checks the sizes of spot and point lights
        // sorts them out and takes the max amount if the size exceeds the limit
        std::vector<PointLight> closestPointLights;
        std::vector<SpotLight> closestSpotLights;
        if (lights.pointLights.size() > MAX_POINT_LIGHTS) {
            closestPointLights = selectPointLights(lights.pointLights);
        }
        if (lights.spotLights.size() >= MAX_SPOT_LIGHTS)
            closestSpotLights = selectSpotLights(lights.spotLights, lights.cameraLight);
        else {
            closestSpotLights = lights.spotLights;
            closestSpotLights.push_back(lights.cameraLight);
        }
        for (auto& mesh : meshes) {
            mesh.draw(projection, view, modelMatrix, lights.sun, closestSpotLights,
                closestPointLights.empty() ? lights.pointLights : closestPointLights);
        }
    }

    glm::vec3 closestPointOnAABB(const glm::vec3& point) {
        return glm::clamp(point, AABBMin, AABBMax);
    }


private:
#include <tuple>
    static constexpr uint MAX_POINT_LIGHTS = 15;
    static constexpr uint MAX_SPOT_LIGHTS = 15;
    int mesh_step_size = 30; // Controls mesh triangle density/detail
    float height_scale = 0.5f; // Controls height exaggeration


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
            AABBMax = glm::max(AABBMax, positions[i]);
            AABBMin = glm::min(AABBMin, positions[i]);

            // create Mesh and store it
            meshes.emplace_back(GL_TRIANGLES, shader, vertices, indices, origin, orientation);

            // set model name based on the filename stem
            name = path.stem().string();
        }
        // origin = glm::vec3((min.x + max.x) / 2.0f, min.y, (min.z + max.z) / 2.0f);

        std::cout << "Loaded model: " << path << "\n"
            << "Origin: (" << origin.x << ", " << origin.y
            << ", " << origin.z << ")\n"
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
        auto terrainMeshes = map.GenHeightMap(terrain, mesh_step_size, height_scale, shader);
        for (auto& mesh : terrainMeshes) {
            meshes.push_back(mesh);
        }
        name = "Terrain";
        std::cout << "Loaded heightmap: resources/textures/heights.png" << std::endl;
    }

    std::vector<PointLight> selectPointLights(const std::vector<PointLight>& lights) {
        // selects n=MAX_POINT_LIGHTS closest to the position of our Model object
        std::vector<PointLight> sorted = lights;
        glm::vec3 objectPos = glm::vec3(modelMatrix[3]);
        std::sort(sorted.begin(), sorted.end(),
            [objectPos](const PointLight& a, PointLight const& b) {
                float distA = glm::distance(a.position, objectPos);
                float distB = glm::distance(b.position, objectPos);
                return distA < distB;
            });
        sorted.resize(MAX_POINT_LIGHTS);
        return sorted;
    }

    std::vector<SpotLight> selectSpotLights(const std::vector<SpotLight>& lights,
        const SpotLight& cameraLight) {
        // selects n=MAX_SPOT_LIGHTS closest to the position of our Model object
        std::vector<SpotLight> sorted = lights;
        sorted.push_back(cameraLight);
        glm::vec3 objectPos = glm::vec3(modelMatrix[3]);
        std::sort(sorted.begin(), sorted.end(),
            [objectPos](const SpotLight& a, SpotLight const& b) {
                float distA = glm::distance(a.position, objectPos);
                float distB = glm::distance(b.position, objectPos);
                return distA < distB;
            });
        sorted.resize(MAX_SPOT_LIGHTS);
        return sorted;
    }
};

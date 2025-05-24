//
// Created by dimonchik on 5/17/25.
//

#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H
#include <opencv2/opencv.hpp>

#include "Mesh.hpp"
#include "assert.h"
#include "ShaderProgram.hpp"



class HeightMap {
#include <tuple>
    // Mesh GenHeightMap(const cv::Mat& hmap, const unsigned int mesh_step_size);
    glm::vec2 get_subtex_st(const int x, const int y)
    {
        return glm::vec2(x * 1.0f / 16, y * 1.0f / 16);
    }

    // choose subtexture based on height
    glm::vec2 get_subtex_by_height(float height)
    {
        if (height > 0.9)
            return get_subtex_st(2, 11); //snow
        else if (height > 0.8)
            return get_subtex_st(3, 11); //ice
        else if (height > 0.5)
            return get_subtex_st(0, 14); //rock
        else if (height > 0.3)
            return get_subtex_st(2, 15); //soil
        else
            return get_subtex_st(0, 11); //grass
    }
    public:
    HeightMap(){};
    std::tuple<std::vector<Vertex>, std::vector<GLuint>> GenHeightMap(const cv::Mat &hmap, const unsigned int mesh_step_size) {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        glm::vec3 v;
        glm::vec4 c;

        std::cout << "Note: heightmap size:" << hmap.size << ", channels: " << hmap.channels() << std::endl;

        if (hmap.channels() != 1) {
            std::cerr << "WARN: requested 1 channel, got: " << hmap.channels() << std::endl;
        }

        // Create heightmap mesh from TRIANGLES in XZ plane, Y is UP (right hand rule)
        //
        //   3-----2
        //   |    /|
        //   |  /  |
        //   |/    |
        //   0-----1
        //
        //   012,023
        //

        for (unsigned int x_coord = 0; x_coord < (hmap.cols - mesh_step_size); x_coord += mesh_step_size)
        {
            for (unsigned int z_coord = 0; z_coord < (hmap.rows - mesh_step_size); z_coord += mesh_step_size) {
                // Get The (X, Y, Z) Value For The Bottom Left Vertex = 0
                glm::vec3 p0(x_coord, hmap.at<uchar>(cv::Point(x_coord, z_coord)), z_coord);
                // Get The (X, Y, Z) Value For The Bottom Right Vertex = 1
                glm::vec3 p1(x_coord + mesh_step_size, hmap.at<uchar>(cv::Point(x_coord + mesh_step_size, z_coord)), z_coord);
                // Get The (X, Y, Z) Value For The Top Right Vertex = 2
                glm::vec3 p2(x_coord + mesh_step_size, hmap.at<uchar>(cv::Point(x_coord + mesh_step_size, z_coord + mesh_step_size)), z_coord + mesh_step_size);
                // Get The (X, Y, Z) Value For The Top Left Vertex = 3
                glm::vec3 p3(x_coord, hmap.at<uchar>(cv::Point(x_coord, z_coord + mesh_step_size)), z_coord + mesh_step_size);

                // Get max normalized height for tile, set texture accordingly
                // Grayscale image returns 0..256, normalize to 0.0f..1.0f by dividing by 256
                float max_h = std::max(hmap.at<uchar>(cv::Point(x_coord, z_coord)) / 256.0f,
                              std::max(hmap.at<uchar>(cv::Point(x_coord, z_coord + mesh_step_size)) / 256.0f,
                              std::max(hmap.at<uchar>(cv::Point(x_coord + mesh_step_size, z_coord + mesh_step_size)) / 256.0f,
                                       hmap.at<uchar>(cv::Point(x_coord + mesh_step_size, z_coord)) / 256.0f
                                )));

                // Get texture coords in vertices, bottom left of geometry == bottom left of texture
                glm::vec2 tc0 = get_subtex_by_height(max_h);
                glm::vec2 tc1 = tc0 + glm::vec2(1.0f / 16, 0.0f);       //add offset for bottom right corner
                glm::vec2 tc2 = tc0 + glm::vec2(1.0f / 16, 1.0f / 16);  //add offset for top right corner
                glm::vec2 tc3 = tc0 + glm::vec2(0.0f, 1.0f / 16);       //add offset for bottom leftcorner

                // normals for both triangles, CCW
                glm::vec3 n1   = glm::normalize(glm::cross(p1-p0, p2-p0)); // for p1
                glm::vec3 n2   = glm::normalize(glm::cross(p2-p0, p3-p0)); // for p3
                glm::vec3 navg = glm::normalize(n1+n2);                 // average for p0, p2 - common

                //place vertices and ST to mesh
                vertices.emplace_back(Vertex{p0, navg, tc0});
                vertices.emplace_back(Vertex{p1, n1,   tc1});
                vertices.emplace_back(Vertex{p2, navg, tc2});
                vertices.emplace_back(Vertex{p3, n2,   tc3});

                // place indices
                // indices.emplace_back(0, 1, 2, 0, 2, 3);
                indices.insert(indices.end(), {0, 1, 2, 0, 2, 3});
            }
        }


        std::tuple indices_vertices(vertices, indices);

        return indices_vertices;
    }
};



#endif //HEIGHTMAP_H

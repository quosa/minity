#pragma once

#define MINITY_SCENE_TYPES_ONLY
#include "old_scene.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <strstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <memory>

namespace minity
{

struct modelImporter
{
public:
    modelImporter() : model_(std::make_shared<old_model>()) {}
    ~modelImporter() { model_.reset(); }
    std::shared_ptr<old_model> load(const std::string &path, bool reverseWinding=false);
    std::shared_ptr<old_model> loadFromString(const std::string &modelString, bool reverseWinding=false);
private:
    void handleLine(const std::string &line);
    void alignFaces(bool reverseWinding);
    std::shared_ptr<old_model> model_{nullptr};
};

// adapted originally from:
// https://github.com/OneLoneCoder/videos/blob/master/OneLoneCoder_olcEngine3D_Part3.cpp
// load the mesh from an obj file
// supports vertices, faces, normals
// and texture coordinates
// might throw runtime exception
std::shared_ptr<old_model> modelImporter::load(const std::string &path, bool reverseWinding)
{
    if (model_)
    {
        model_.reset();
        model_ = std::make_shared<old_model>();
    }
    std::ifstream f(path);
    if (!f.is_open())
    {
        // std::cerr << "Trouble opening file: '" << path << "'" << std::endl;
        throw std::runtime_error("Trouble opening file: '" + path + "'");
    }
    std::string line;
    while (!f.eof())
    {
        // std::cout << line << std::endl;
        std::getline(f, line);
        handleLine(line);
    }
    alignFaces(reverseWinding);
    // model_->printModelInfo();
    return model_;
}

// might throw runtime exception
std::shared_ptr<old_model> modelImporter::loadFromString(const std::string &modelString, bool reverseWinding)
{
    if (model_)
    {
        model_.reset();
        model_ = std::make_shared<old_model>();
    }
    std::istringstream iss(modelString);
    std::string line;
    while (std::getline(iss, line)) {
        // std::cout << line << std::endl;
        handleLine(line);
    }
    alignFaces(reverseWinding);
    // model_->printModelInfo();
    return model_;
}

// utility to align vertices, normals
// and texture coordinates for easy access
void modelImporter::alignFaces(bool reverseWinding)
{
    // (deep-)copy the unsorted arrays
    std::vector<vec3> _vertices = model_->vertices;
    std::vector<vec3> _normals = model_->normals;
    std::vector<vec2> _textureCoordinates = model_->textureCoordinates;
    std::vector<std::vector<int>> _faces = model_->faces; // v1_idx, v2_idx, v3_idx, ...

    // reset the old_model arrays
    model_->vertices.clear();
    model_->normals.clear();
    model_->textureCoordinates.clear();
    model_->faces.clear();

    // populate the arrays back in order
    // so that each have same id for same vertex
    int i = 0;
    for (auto vnt : _faces)
    {

        model_->vertices.insert( model_->vertices.end(), { _vertices[vnt[0]], _vertices[vnt[3]], _vertices[vnt[6]]});
        if (reverseWinding)
        {
            int n = model_->vertices.size();
            std::swap( model_->vertices[n-2], model_->vertices[n-1]);
        }

        if (model_->hasNormals)
        {
            model_->normals.insert( model_->normals.end(), {_normals[vnt[1]], _normals[vnt[4]], _normals[vnt[7]]});
            if (reverseWinding)
            {
                int n = model_->normals.size();
                std::swap( model_->normals[n-2], model_->normals[n-1]);
            }
        }

        if (model_->hasTextureCoordinates)
        {
            model_->textureCoordinates.insert( model_->textureCoordinates.end(),
                { _textureCoordinates[vnt[2]], _textureCoordinates[vnt[5]], _textureCoordinates[vnt[8]]}
            );
            if (reverseWinding)
            {
                int n = model_->textureCoordinates.size();
                std::swap( model_->textureCoordinates[n-2], model_->textureCoordinates[n-1]);
            }
        }
        std::vector<int> face{ i, i+1, i+2 };
        model_->faces.insert( model_->faces.end(), face);
        i += 3;
    }
    model_->numFaces = model_->faces.size();
}

// utility to handle a obj file line
void modelImporter::handleLine(const std::string &line)
{
    std::strstream s;
    s << line;

    char junk;

    // TODO: wait for c++20 starts_with
    if (line.rfind("v ", 0) == 0) // vertex
    {
        // e.g. v 0.123 0.234 0.345 1.0
        vec3 v;
        s >> junk >> v.x >> v.y >> v.z;
        model_->vertices.push_back(v);
    }
    else if (line.rfind("vn ", 0) == 0) // normal
    {
        // e.g. vn 0.707 0.000 0.707
        vec3 v;
        s >> junk >> junk >> v.x >> v.y >> v.z;
        model_->normals.push_back(v);
    }
    else if (line.rfind("vt ", 0) == 0) // texture coordinates
    {
        // e.g. vt 0.500 1 [0]
        vec2 v;
        s >> junk >> junk >> v.u >> v.v;
        if (v.u > 1.0f && v.u <= 2.0f)
            v.u -= 1.0f; // likely indicates tiling
        if (v.v > 1.0f && v.v <= 2.0f)
            v.v -= 1.0f; // likely indicates tiling
        if (v.u < 0.0f && v.u >= -1.0f)
            v.u += 1.0f; // likely indicates tiling
        if (v.v < 0.0f && v.v >= -1.0f)
            v.v += 1.0f; // likely indicates tiling
        model_->textureCoordinates.push_back(v);
    }
    else if (line.rfind("f ", 0) == 0) // face
    {
        // e.g. f 1/11/111 2/22/222 3/33/333 (there are other types!)
        std::string vertexDetails;
        int vrt[50]{0}; // vertex indices
        int nrm[50]{0}; // normal indices
        int tex[50]{0}; // texture cordinate indices
        int i = 0;

        s >> junk;

        while (s >> vertexDetails)
        {
            std::vector<int> indices;
            size_t last = 0;
            size_t next = 0;
            // handle all possible scenarios here: 1, 1/2, 1/2/3, 1//3
            while ((next = vertexDetails.find('/', last)) != std::string::npos)
            {
                int idx = 0; // case f 1//3
                if ((next - last) > 0)
                    idx = std::stoi(vertexDetails.substr(last, next - last));
                indices.push_back(idx);
                last = next + 1;
            }
            indices.push_back(std::stoi(vertexDetails.substr(last)));

            // std::cout << "indices: [";
            // for (auto elem : indices) { std::cout << " " << elem; };
            // std::cout << " ]" << std::endl;

            auto n = std::count(vertexDetails.begin(), vertexDetails.end(), '/');
            switch (n)
            {
            case 0: // just vertex indices: f 1 2 3
                vrt[i] = indices.at(0);
                nrm[i] = 0; // ends up as -1
                tex[i] = 0; // ends up as -1
                break;
            case 1: // vertex and texture coordinates: f 1/7 2/8 3/9
                vrt[i] = indices.at(0);
                tex[i] = indices.at(1);
                nrm[i] = 0; // ends up as -1
                model_->hasTextureCoordinates = true;
                break;
            case 2: // vertex, texture coordinates and normals: f 1/7/12 2/8/13 3/9/14
                vrt[i] = indices.at(0);
                tex[i] = indices.at(1);
                nrm[i] = indices.at(2);
                model_->hasNormals = true;
                model_->hasTextureCoordinates = (tex[i] != 0);
                break;
            default:
                // std::cerr << "Trouble reading face vertex information: " << vertexDetails << std::endl;
                throw std::runtime_error("Trouble reading face vertex information: " + vertexDetails); // we don't support anything else...
            }
            i++;
        }

        // pack the scattered indices to each face in v1, n1, t1, v2, n2, t2...
        std::vector<int> face;
        for (int j : {0, 1, 2})
        {
            face.insert( face.end(), { vrt[j] - 1, nrm[j] - 1, tex[j] -1} );
        }
        model_->faces.push_back(face);
        model_->numFaces++;

        if (i == 4)
        {
            // Break the 4 vertex manually
            // TODO: this is fragile!
            face.clear();
            for (int k : {0, 2, 3})
            {
                face.insert( face.end(), { vrt[k] - 1, nrm[k] - 1, tex[k] -1} );
            }
            model_->faces.push_back(face);
            model_->numFaces++;
        }
        else if (i>4)
        {
            std::cerr << "Warning, clipping a polygon of " << i << " vertices !!!" << std::endl;
        }

    }
}

} // minity

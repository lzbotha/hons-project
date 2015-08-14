#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/Exporter.hpp>

#include <iostream>
#include <math.h>
#include <unordered_set>
#include <unordered_map>

#include "../include/mesh.h"
#include "../include/utils.h"

mesh::mesh() : neighbouring_triangles(std::unordered_map<int, std::unordered_set<int>>()) {

}

bool mesh::import_from_file(const std::string& filepath) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filepath,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenNormals |
        aiProcess_PreTransformVertices | // TODO: figure out the node graph
        aiProcess_FixInfacingNormals |
        aiProcess_FindDegenerates
    );

    if(!scene)
    {
        std::cerr << importer.GetErrorString() << std::endl;
        return false;
    }

    this->scene = importer.GetOrphanedScene();

    return true;
}

bool mesh::export_to_file(const std::string& format, const std::string& filepath) {
    Assimp::Exporter exporter;

    if(AI_SUCCESS == exporter.Export(this->scene, format, filepath)) {
        return true;
    }

    std::cerr << exporter.GetErrorString() << std::endl;
    return false;
}

bool mesh::prune() {
    aiVector3D n(0.0f, 1.0f, 0.0f);

    if (scene->mNumMeshes != 1)
        return false;

    aiMesh * mesh = scene->mMeshes[0];

    int num_new = 0;
    aiFace * new_faces = new aiFace[mesh->mNumFaces];
    aiVector3D * new_normals = new aiVector3D[mesh->mNumFaces];

    for (int i = 0; i < mesh->mNumFaces; ++i) {
        float angle = utils::angle_between(
            n, 
            get_face_normal(mesh->mFaces[i])
        );

        if (angle < 0.785398163f / 2) {

            new_faces[num_new] = mesh->mFaces[i];
            new_normals[num_new] = mesh->mNormals[i];
            ++num_new;
        }
    }

    delete[] mesh->mFaces;
    delete[] mesh->mNormals;

    mesh->mNumFaces = num_new;
    mesh->mFaces = std::move(new_faces);
    mesh->mNormals = std::move(new_normals);

    return true;
}

aiVector3D mesh::get_face_normal(const aiFace & face) {
    aiVector3D n(0,0,0);

    for (int i = 0; i < face.mNumIndices; ++i) {
        aiMesh * mesh = scene->mMeshes[0];
        n += mesh->mNormals[face.mIndices[i]];
    }

    return n /= face.mNumIndices;
}

void mesh::setup_neighbouring_triangles() {
    using namespace std;
    cout << "triangle neighbour start" << endl;
    aiMesh * mesh = scene->mMeshes[0];

    // make a map from edges to triangles
    // TODO: consider making the key an unordered_set/pair
    unordered_map<string, unordered_set<int>> edge_to_triangle_map;

    for (int f_index = 0 ; f_index < mesh->mNumFaces; ++f_index) {
        aiFace & f = mesh->mFaces[f_index];

        for (int v = 0; v < f.mNumIndices; ++v) {
            int v1 = f.mIndices[v];
            int v2 = f.mIndices[(v + 1) % f.mNumIndices];
            
            // If the edge is in the set append this triangle
            if (edge_to_triangle_map.find(to_string(v1) + ":" + to_string(v2)) != edge_to_triangle_map.end()) {
                edge_to_triangle_map[to_string(v1) + ":" + to_string(v2)].emplace(f_index);
            }
            // If the edge is in the set append this triangle
            else if (edge_to_triangle_map.find(to_string(v2) + ":" + to_string(v1)) != edge_to_triangle_map.end()) {
                edge_to_triangle_map[to_string(v2) + ":" + to_string(v1)].emplace(f_index);
            }
            // If the edge does not appear at all in the mapping add it
            else {
                edge_to_triangle_map.emplace(
                    to_string(v1) + ":" + to_string(v2),
                    unordered_set<int>({f_index})
                );
            }
        }

        neighbouring_triangles.emplace(f_index, unordered_set<int>());
    }

    for (int f_index = 0 ; f_index < mesh->mNumFaces; ++f_index) {
        aiFace & f = mesh->mFaces[f_index];

        for (int v = 0; v < f.mNumIndices; ++v) {
            int v1 = f.mIndices[v];
            int v2 = f.mIndices[(v + 1) % f.mNumIndices];

            if (edge_to_triangle_map.find(to_string(v1) + ":" + to_string(v2)) != edge_to_triangle_map.end()) {
                for (int t : edge_to_triangle_map[to_string(v1) + ":" + to_string(v2)]) {
                    neighbouring_triangles[f_index].emplace(t);
                }
            }
            else {
                for (int t: edge_to_triangle_map[to_string(v2) + ":" + to_string(v1)]) {
                    neighbouring_triangles[f_index].emplace(t);
                }
            }
        }
    }    

    cout << "triangle neighbour end" << endl;
}
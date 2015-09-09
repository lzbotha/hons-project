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

void mesh::delete_faces(std::unordered_set<int> & to_delete) {
    aiMesh * mesh = scene->mMeshes[0];

    int num_new = 0;
    aiFace * new_faces = new aiFace[mesh->mNumFaces];

    for (int i = 0; i < mesh->mNumFaces; ++i) {
        if (to_delete.find(i) == to_delete.end()) {

            new_faces[num_new] = mesh->mFaces[i];
            ++num_new;
        }
    }

    delete[] mesh->mFaces;

    mesh->mNumFaces = num_new;
    mesh->mFaces = std::move(new_faces);
}

bool mesh::prune() {
    aiVector3D n(0.0f, 1.0f, 0.0f);

    if (scene->mNumMeshes != 1)
        return false;

    aiMesh * mesh = scene->mMeshes[0];

    int num_new = 0;
    aiFace * new_faces = new aiFace[mesh->mNumFaces];

    for (int i = 0; i < mesh->mNumFaces; ++i) {
        float angle = utils::angle_between(
            n, 
            get_face_normal(mesh->mFaces[i])
        );

        if (angle < 0.785398163f / 2) {

            new_faces[num_new] = mesh->mFaces[i];
            ++num_new;
        }
    }

    delete[] mesh->mFaces;

    mesh->mNumFaces = num_new;
    mesh->mFaces = std::move(new_faces);

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

bool mesh::setup_neighbouring_triangles() {
    using namespace std;
    aiMesh * mesh = scene->mMeshes[0];

    // make a map from edges to triangles
    unordered_map<string, unordered_set<int>> edge_to_triangle_map;

    for (int f_index = 0 ; f_index < mesh->mNumFaces; ++f_index) {
        aiFace & f = mesh->mFaces[f_index];

        for (int v = 0; v < f.mNumIndices; ++v) {
            int v1 = f.mIndices[v];
            int v2 = f.mIndices[(v + 1) % f.mNumIndices];

            string v_1 = to_string(mesh->mVertices[v1].x) + " " + to_string(mesh->mVertices[v1].y) + " " + to_string(mesh->mVertices[v1].z);
            string v_2 = to_string(mesh->mVertices[v2].x) + " " + to_string(mesh->mVertices[v2].y) + " " + to_string(mesh->mVertices[v2].z);
            
            // If the edge is in the set append this triangle
            if (edge_to_triangle_map.find(v_1 + ":" + v_2) != edge_to_triangle_map.end()) {
                edge_to_triangle_map[v_1 + ":" + v_2].emplace(f_index);
            }

            // If the edge is in the set append this triangle
            else if (edge_to_triangle_map.find(v_2 + ":" + v_1) != edge_to_triangle_map.end()) {
                edge_to_triangle_map[v_2 + ":" + v_1].emplace(f_index);
            }

            // If the edge does not appear at all in the mapping add it
            else {
                edge_to_triangle_map.emplace(
                    v_1 + ":" + v_2,
                    unordered_set<int>({f_index})
                );
            }
        }

        neighbouring_triangles.emplace(f_index, unordered_set<int>());
    }

    // Iterate over triangles again using the edge_to_triangle_map to add neighbouring triangles
    for (int f_index = 0 ; f_index < mesh->mNumFaces; ++f_index) {
        aiFace & f = mesh->mFaces[f_index];

        for (int v = 0; v < f.mNumIndices; ++v) {
            int v1 = f.mIndices[v];
            int v2 = f.mIndices[(v + 1) % f.mNumIndices];

            string v_1 = to_string(mesh->mVertices[v1].x) + " " + to_string(mesh->mVertices[v1].y) + " " + to_string(mesh->mVertices[v1].z);
            string v_2 = to_string(mesh->mVertices[v2].x) + " " + to_string(mesh->mVertices[v2].y) + " " + to_string(mesh->mVertices[v2].z);

            if (edge_to_triangle_map.find(v_1 + ":" + v_2) != edge_to_triangle_map.end()) {
                for (int t : edge_to_triangle_map[v_1 + ":" + v_2]) {
                    if (t != f_index)
                        neighbouring_triangles[f_index].emplace(t);
                }
            }
            else {
                for (int t: edge_to_triangle_map[v_2 + ":" + v_1]) {
                    if (t != f_index)
                        neighbouring_triangles[f_index].emplace(t);
                }
            }
        }
    }

    return true;
}

void mesh::fill(
    int f,
    std::unordered_set<int> & faces,
    std::unordered_set<int> & chunk
    )
{
    // if all the neighbours of this face are already in chunk return
    for (int nf : neighbouring_triangles[f]) {
        if (faces.find(nf) != faces.end()) {
            // recurse further
            faces.erase(nf);
            chunk.insert(nf);
            this->fill(nf, faces, chunk);
        }
    }
}

bool mesh::cull_chunks(int min_size) {
    using namespace std;

    if (this->neighbouring_triangles.size() == 0)
        return false;

    // Put all faces into a set
    unordered_set<int> faces;
    aiMesh * mesh = scene->mMeshes[0];

    // TODO: use and stl function or something better for this
    for (int i = 0; i < mesh->mNumFaces; ++i) {
        faces.emplace(i);
    }

    int num_faces = faces.size();

    unordered_set<int> to_delete;

    while (!faces.empty()) {

        unordered_set<int> chunk;

        // select a face
        int f = -1;
        std::unordered_set<int>::iterator iter = faces.begin();
        if (iter != faces.end())
            f = *iter;
        else{
            cerr << "No remaining faces" << endl;
            cerr << faces.size() << endl;
            break;
        }

        chunk.insert(f);
        faces.erase(f);

        // iterate over all its neighbours adding it to a local set
        this->fill(f, faces, chunk);

        // once all its neighbours have been iterated over delete them if
        if (chunk.size() < min_size) {
            to_delete.insert(chunk.begin(), chunk.end());
        }
    }

    this->delete_faces(to_delete);

    return true;
}
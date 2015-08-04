#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/Exporter.hpp>

#include <iostream>
#include <math.h>

#include "../include/mesh.h"
#include "../include/utils.h"

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
    // std::cout << n[0] << " " << n[1] << " " << n[2] << std::endl;

    if (scene->mNumMeshes != 1)
        return false;

    aiMesh * mesh = scene->mMeshes[0];

    int num_new = 0;
    aiFace * new_faces = new aiFace[mesh->mNumFaces];
    aiVector3D * new_normals = new aiVector3D[mesh->mNumFaces];

    for (int i = 0; i < mesh->mNumFaces; ++i) {
        // std::cout << i << std::endl;
        float angle = utils::angle_between(
            n, 
            getFaceNormal(mesh->mFaces[i])
        );
        // std::cout << angle << std::endl;
        // std::cout << mesh->mNormals[i][0] << " " << mesh->mNormals[i][1] << " " << mesh->mNormals[i][2] << "\tangle: " << angle << std::endl;

        if (angle < 0.785398163f / 2) {

            // std::cout << mesh->mNormals[i][0] << " " << mesh->mNormals[i][1] << " " << mesh->mNormals[i][2] << std::endl;

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

aiVector3D mesh::getFaceNormal(const aiFace & face) {
    aiVector3D n(0,0,0);

    for (int i = 0; i < face.mNumIndices; ++i) {
        aiMesh * mesh = scene->mMeshes[0];
        n += mesh->mNormals[face.mIndices[i]];
    }

    return n /= face.mNumIndices;
}
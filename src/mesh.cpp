#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/Exporter.hpp>

#include <iostream>

#include "../include/mesh.h"

bool mesh::DoTheImportThing( const std::string& pFile)
{
    using namespace std;
    // Create an instance of the Importer class
    Assimp::Importer importer;
    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll 
    // propably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile( pFile,
    aiProcess_CalcTangentSpace |
    aiProcess_Triangulate |
    aiProcess_JoinIdenticalVertices |
    aiProcess_SortByPType);
    // If the import failed, report it
    if( !scene)
    {
        // DoTheErrorLogging( importer.GetErrorString());
        return false;
    }
    // Now we can access the file's contents. 

    cout << "num meshes: " << scene->mNumMeshes << endl;
    aiMesh* mesh = scene->mMeshes[0];

    for (int i = 0; i < mesh->mNumFaces; ++i)
        cout << ".";

    mesh->mNumFaces = mesh->mNumFaces / 2;

    Assimp::Exporter exporter;
    exporter.Export(scene, "stl", "meshes/assimp-test.stl");

    // We're done. Everything will be cleaned up by the importer destructor
    return true;
}

bool mesh::import_from_file(const std::string& filepath) {
    // 
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
        // DoTheErrorLogging( importer.GetErrorString());
        return false;
    }

    this->scene = importer.GetOrphanedScene();

    return true;
}

bool mesh::export_to_file(const std::string& format, const std::string& filepath) {
    Assimp::Exporter exporter;
    exporter.Export(this->scene, format, filepath);

    // TODO: handle the aiError returned by Export
    return false;
}

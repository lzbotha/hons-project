#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/Exporter.hpp>

#include <iostream>
#include <math.h>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <cmath>

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

    aiScene * temp;
    aiCopyScene(this->scene, &temp);
    keep_faces(this->walkable_faces, temp);

    if(AI_SUCCESS == exporter.Export(temp, format, filepath)) {
        return true;
    }

    std::cerr << exporter.GetErrorString() << std::endl;
    return false;

    delete temp;
}

bool mesh::is_walkable(int f) {
    return walkable_faces.find(f) != walkable_faces.end();
}

void mesh::keep_faces(std::unordered_set<int> & to_keep, aiScene * s) {
    aiMesh * mesh = s->mMeshes[0];

    int num_new = 0;
    aiFace * new_faces = new aiFace[mesh->mNumFaces];

    for (int i = 0; i < mesh->mNumFaces; ++i) {
        if (to_keep.find(i) != to_keep.end()) {
            new_faces[num_new] = mesh->mFaces[i];
            ++num_new;
        }
    }

    delete[] mesh->mFaces;

    mesh->mNumFaces = num_new;
    mesh->mFaces = std::move(new_faces);
}

void mesh::delete_faces(std::unordered_set<int> & to_delete) {
    for (int f : to_delete)
        this->walkable_faces.erase(f);
}

bool mesh::prune() {
    // TODO: make this more generic and bad in general
    aiVector3D n(0.0f, 1.0f, 0.0f);

    if (scene->mNumMeshes != 1)
        return false;

    aiMesh * mesh = scene->mMeshes[0];

    for (int i = 0; i < mesh->mNumFaces; ++i) {
        float angle = utils::angle_between(
            n, 
            get_face_normal(mesh->mFaces[i])
        );

        // TODO: don't hard code this either
        if (angle < 0.785398163f / 2) {
            this->walkable_faces.insert(i);
        }
    }

    return true;
}

float mesh::distance(int face1, int face2) {
    aiMesh * mesh = scene->mMeshes[0];
    
    aiFace f1 = mesh->mFaces[face1];
    aiVector3D f1m(0,0,0);
    for (int i = 0; i < f1.mNumIndices; ++i)
        f1m += mesh->mVertices[f1.mIndices[i]];
    f1m /= f1.mNumIndices;

    aiFace f2 = mesh->mFaces[face2];
    aiVector3D f2m(0,0,0);
    for (int i = 0; i < f2.mNumIndices; ++i)
        f2m += mesh->mVertices[f2.mIndices[i]];
    f2m /= f2.mNumIndices;

    return sqrt(
        pow(f1m.x - f2m.x, 2) +
        pow(f1m.y - f2m.y, 2) +
        pow(f1m.z - f2m.z, 2)
    );
}

void mesh::spill(int origin_face, int current_face, std::unordered_set<int> & origin_chunk, float dist, std::vector<int> path, std::unordered_set<int> & visited, std::unordered_set<int> & to_add) {

    if (origin_chunk.find(current_face) != origin_chunk.end()){
        // std::cout << "X";
        return;
    }

    if (distance(origin_face, current_face) > dist){
        // std::cout << "D";
        return;
    }

    // if the current face is walkable
    if (walkable_faces.find(current_face) != walkable_faces.end()) {

        if (origin_chunk.find(current_face) != origin_chunk.end()){
            std::cout << "The sky is falling" << std::endl;
        }

        // add all faces
        for (int f : path)
            to_add.insert(f);

        // std::cout << "adding some faces: " << path.size() << std::endl;
        // std::cout << "A";
        return;
    }

    // std::cout << "X";

    for (int nf : neighbouring_triangles[current_face]) {
        if (origin_face == nf)
            continue;

        if (visited.find(nf) != visited.end())
            continue;

        // TODO: tidy this up since it is not necessary as the method stub uses copy semantics anyway
        // std::cout << "." << std::endl;
        std::vector<int> new_path = path;
        new_path.emplace_back(current_face);

        visited.insert(current_face);

        // std::cout << new_path.size() << std::endl;
        // std::cout << "dist: " << distance(origin_face, current_face) << std::endl;
        spill(origin_face, nf, origin_chunk, dist, new_path, visited, to_add);
    }
}

bool mesh::rejoin_chunks(float distance) {
    using namespace std;

    if (this->neighbouring_triangles.size() == 0)
        return false;

    if (scene->mNumMeshes != 1)
        return false;

    // setup all chunks
    vector<unordered_set<int>> chunks;

    // Put all walkable faces into a set
    unordered_set<int> faces = this->walkable_faces;
    aiMesh * mesh = scene->mMeshes[0];

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

        // once the chunk has been populated add it to the vector of chunks
        chunks.emplace_back(chunk);
    }

    // int prgress = 0;
    // int size = walkable_faces.size();

    unordered_set<int> to_add;

    for (unordered_set<int> & c : chunks) {
        for (int f : c) {
            // ++prgress;
            // std::cout << prgress << " / " << size << std::endl;
            // std::cout << ".";

            for (int nf : neighbouring_triangles[f]){
                unordered_set<int> visited;
                spill(f, nf, c, distance, vector<int>(), visited, to_add);
            }
        }
        // prgress += c.size();
        // std::cout << prgress << " / " << size << std::endl;
    }

    for (int f : to_add)
        walkable_faces.insert(f);

    return true;
}

bool mesh::keep_largest_chunk() {
    using namespace std;

    if (this->neighbouring_triangles.size() == 0)
        return false;

    if (scene->mNumMeshes != 1)
        return false;

    // setup all chunks
    vector<unordered_set<int>> chunks;

    // Put all walkable faces into a set
    unordered_set<int> faces = this->walkable_faces;
    aiMesh * mesh = scene->mMeshes[0];

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

        // once the chunk has been populated add it to the vector of chunks
        chunks.emplace_back(chunk);
    }

    cout << "Number of chunks: " << chunks.size() << endl;

    int size = 0;
    int index = -1;

    for (int i = 0; i < chunks.size(); ++i) {
        cout << "chunk: " << i << endl;
        if (chunks[i].size() > size){
            size = chunks[i].size();
            index = i;
            cout << "new size: " << size << endl;
            cout << "new index: " << index << endl;
        }
    }

    this->walkable_faces = std::move(chunks[index]);

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

    if (scene->mNumMeshes != 1)
        return false;

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
    // TODO: refactor this since all this code is duplicated

    using namespace std;

    if (this->neighbouring_triangles.size() == 0)
        return false;

    if (scene->mNumMeshes != 1)
        return false;

    // Put all walkable faces into a set
    unordered_set<int> faces = this->walkable_faces;
    aiMesh * mesh = scene->mMeshes[0];

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
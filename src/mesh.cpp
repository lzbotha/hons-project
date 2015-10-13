#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/Exporter.hpp>

#include <iostream>
#include <math.h>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <stack>
#include <queue>

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

bool mesh::export_to_file(const std::string& format, const std::string& filepath, const std::string & filename) {
    Assimp::Exporter exporter;

    // TODO: fix the memory leak from not deleting temp and temp2
    // Note that aiScene::~aiScene() is not exported by the library

    aiScene * temp;
    aiCopyScene(this->scene, &temp);
    keep_faces(this->walkable_faces, temp);
    auto write_1 = exporter.Export(temp, format, filepath+"w_"+filename);

    aiScene * temp2;
    aiCopyScene(this->scene, &temp2);
    aiMesh * mesh = scene->mMeshes[0];

    std::unordered_set<int> unwalkable;
    for (int i = 0; i < mesh->mNumFaces; ++i){
        // std::cout << "adding face: " << i << std::endl;
        unwalkable.insert(i);
    }

    std::cout << "unwalkable initial size: " << unwalkable.size() << std::endl;
    std::cout << "walkable size: " << walkable_faces.size() << std::endl;

    for (int f : this->walkable_faces){
        unwalkable.erase(f);
        // std::cout << "removing face: " << f << std::endl;
    }

    // for (int f : unwalkable){
    //     std::cout << "not walkable: " << f << std::endl;
    // }

    std::cout << "unwalkable final size: " << unwalkable.size() << std::endl;

    keep_faces(unwalkable, temp2);

    // std::cout << "There are : " << temp2->mMeshes[0]->mNumFaces << std::endl;

    // auto write_2 = exporter.Export(temp2, format, filepath+"u_"+filename);

    // if(AI_SUCCESS == write_1 and AI_SUCCESS == write_2) {
    if(AI_SUCCESS == write_1) {
        return true;
    }

    std::cerr << exporter.GetErrorString() << std::endl;
    return false;
}

bool mesh::is_walkable(int f) {
    return walkable_faces.find(f) != walkable_faces.end();
}

void mesh::keep_faces(std::unordered_set<int> & to_keep, aiScene * s) {
    aiMesh * mesh = s->mMeshes[0];

    int num_new = 0;
    aiFace * new_faces = new aiFace[mesh->mNumFaces];

    for (int i = 0; i < mesh->mNumFaces; ++i) {
        // If this face is in to_keep keep it
        if (to_keep.count(i) > 0) {
            // std::cout << "keeping face: " << i << std::endl;
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

bool mesh::prune(float delta_angle, aiVector3D n) {
    // TODO: throw an exception earlier if this is the case (probably in the import method)
    if (scene->mNumMeshes != 1)
        return false;

    aiMesh * mesh = scene->mMeshes[0];

    for (int i = 0; i < mesh->mNumFaces; ++i) {
        float angle = utils::angle_between(
            n, 
            get_face_normal(mesh->mFaces[i])
        );

        if (angle < delta_angle) {
            this->walkable_faces.insert(i);
        }
    }

    return true;
}

bool mesh::prune_weighted_gradient(float radius, float gradient, int weighting) {
    using namespace std;

    aiMesh * mesh = scene->mMeshes[0];
    aiVector3D up = aiVector3D(0.0f, 1.0f, 0.0f);

    for (int f = 0; f < mesh->mNumFaces; ++f) {
        float avg_gradient = weighting * utils::angle_between(
            up, 
            get_face_normal(mesh->mFaces[f])
        );

        int count = weighting;
        vector<vector<int>> indices; 
        vector<vector<float>> dists;

        this->get_faces_in_radius(f, radius, indices, dists);

        for (int n : indices[0]){
            avg_gradient += utils::angle_between(
                up, 
                get_face_normal(mesh->mFaces[n])
            );
            ++count;
        }
        avg_gradient /= count;

        if (avg_gradient < gradient)
            this->walkable_faces.insert(f);
    }

    return true;
}

void mesh::get_face_center(int face, aiVector3D & center) {
    center.x = center.y = center.z = 0;

    aiMesh * mesh = scene->mMeshes[0];
    const aiFace & f = mesh->mFaces[face];

    for (int i = 0; i < f.mNumIndices; ++i)
        center += mesh->mVertices[f.mIndices[i]];
    center /= f.mNumIndices;
}

float mesh::distance(int face1, int face2) {
    aiMesh * mesh = scene->mMeshes[0];
    
    aiVector3D f1m(0,0,0);
    this->get_face_center(face1, f1m);

    aiVector3D f2m(0,0,0);
    this->get_face_center(face2, f2m);

    return sqrt(
        pow(f1m.x - f2m.x, 2) +
        pow(f1m.y - f2m.y, 2) +
        pow(f1m.z - f2m.z, 2)
    );
}

struct S
{
    int face;
    float dist;

    S(int face, float dist) : face(face), dist(dist)
    {
    }

    bool operator<(const struct S& other) const
    {
        //Your priority logic goes here
        return dist < other.dist;
    }
};

void mesh::better_spill(int f, std::unordered_set<int> & to_add, float distancee, std::unordered_set<int> & chunk) {
    using namespace std;

    // If this polygon is surrounded by only polygons in the same chunk skip it
    bool surrounded = true;
    for (int n : neighbouring_triangles[f])
        surrounded &= !(chunk.count(n) == 0);

    if (surrounded)
        return;

    unordered_map<int, float> dist;
    unordered_map<int, int> prev;
    priority_queue<S> pq;

    pq.push(S(f, 0.0f));

    while(!pq.empty()) {
        S min = pq.top();
        pq.pop();

        for (int n : neighbouring_triangles[min.face]) {
            S alt(n, distance(f, n));
            // S alt(n, min.dist + 1.0f);

            // Cuttoffs
            // Max step distance is exceeded
            if (alt.dist > distancee)
                continue;

            // Polygon is a part of the current chunk
            // if (chunk.count(n) > 0)
            //     continue;

            if (dist.count(alt.face) == 0) {
                dist.emplace(alt.face, alt.dist);
                prev.emplace(alt.face, min.face);

                pq.push(alt);
            }
            else if (alt.dist < dist[alt.face]) {
                dist.emplace(alt.face, alt.dist);
                prev.emplace(alt.face, min.face);

                pq.push(alt);
            }
        }
    }

    for(auto kv : prev) {
        // If the polygon is walkable AND not in the same chunk as the starting point
        // if ((walkable_faces.count(kv.first) > 0) and chunk.count(kv.first) == 0) {
        if (walkable_faces.count(kv.first) > 0) {
            int curr = kv.first;

            while (curr != f){
                to_add.insert(prev[curr]);

                if (prev.count(curr) > 0)
                    curr = prev[curr];
            }
        }
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

    unordered_set<int> to_add;

    // int size = 0;
    // for (unordered_set<int> & c : chunks) {
    //     size += c.size();
    // }

    // int t_size = size;

    for (unordered_set<int> & c : chunks) {
        for (int f : c) {

            // size--;
            // cout << size << " / " << t_size << endl;
            better_spill(f, to_add, distance, c);
        }
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

    cout << "number of faces: " << faces.size() << endl;

    cout << "before while loop" << endl;
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

        // TODO: figure out if this is just fill and if so replace it
        // iterate over all its neighbours adding it to a local set
        stack<int> to_process;
        to_process.emplace(f);

        // TODO: change this to use .empty instead
        while (to_process.size() > 0) {
            int face = to_process.top();
            to_process.pop();

            // if all the neighbours of this face are already in chunk return
            for (int nf : neighbouring_triangles[face]) {
                if (faces.find(nf) != faces.end()) {
                    // recurse further
                    faces.erase(nf);
                    chunk.insert(nf);
                    // std::cout << faces.size() << std::endl;

                    to_process.emplace(nf);
                }
            }
        }

        // once the chunk has been populated add it to the vector of chunks
        chunks.emplace_back(chunk);
    }

    cout << "Number of chunks: " << chunks.size() << endl;

    int size = 0;
    int index = -1;

    for (int i = 0; i < chunks.size(); ++i) {
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
    // iterate over all its neighbours adding it to a local set
    std::stack<int> to_process;
    to_process.emplace(f);

    // while to_process is not empty
    while (!to_process.empty()) {
        int face = to_process.top();
        to_process.pop();

        // if all the neighbours of this face are already in chunk return
        for (int nf : neighbouring_triangles[face]) {
            if (faces.find(nf) != faces.end()) {
                // recurse further
                faces.erase(nf);
                chunk.insert(nf);

                to_process.emplace(nf);
            }
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

bool cull_chunks(float min_area) {


    return true;
}

void mesh::clear_walkable_surfaces() {
    this->walkable_faces.clear();
}

void mesh::merge_chunks() {
    using namespace std;

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

    // Find the edges of each chunk
    vector<unordered_set<int>> chunks_edges;
    for (unordered_set<int> & c : chunks) {
        unordered_set<int> edges;

        for (int f : c) {
            std::unordered_set<int> & neighbours = neighbouring_triangles[f];
            for (int n : neighbours) {
                if (c.count(n) == 0)
                    edges.insert(n);
            }
        }
        chunks_edges.emplace_back(edges);
    }

    // for each chunk try and link it with each other chunk
}

void mesh::setup_spatial_structures() {
    // TODO: either clear the memory from f_index or make this a constructor only method
    using namespace std;
    
    aiMesh * mesh = scene->mMeshes[0];

    // add all the current flaces to the index
    float * pts = new float[mesh->mNumFaces * 3];
    for (int i = 0; i < mesh->mNumFaces; ++i){
        aiVector3D f(0,0,0);
        this->get_face_center(i, f);

        pts[i*3 + 0] = f.x;
        pts[i*3 + 1] = f.y;
        pts[i*3 + 2] = f.z;
    }

    flann::Matrix<float> dataset(pts, mesh->mNumFaces, 3);
    this->f_index = new flann::Index<flann::L2_Simple<float>>(dataset, flann::KDTreeSingleIndexParams());
    this->f_index->buildIndex();

    // TODO: handle memory cleanup for the index construction
}

int mesh::get_faces_in_radius(
    int face, 
    float radius, 
    std::vector<std::vector<int>> & indices, 
    std::vector<std::vector<float>> & dists
) {
    using namespace std;

    aiVector3D c(0.0f, 0.0f, 0.0f);
    this->get_face_center(face, c);

    float * q = new float[3];
    q[0] = c.x;
    q[1] = c.y;
    q[2] = c.z;
    flann::Matrix<float> query(q,1,3);

    return f_index->radiusSearch(query, indices, dists, radius, flann::SearchParams());
}

int mesh::get_faces_in_radius(const std::vector<std::vector<float>> & positions, float radius, std::vector<std::vector<int>> & indices, std::vector<std::vector<float>> & dists){
    using namespace std;
    float * q = new float[3 * positions.size()];

    for (int i = 0; i < positions.size(); ++i){

        q[i*3 + 0] = positions[i][0];
        q[i*3 + 1] = positions[i][1];
        q[i*3 + 2] = positions[i][2];
    }

    flann::Matrix<float> query(q,positions.size(),3);

    return f_index->radiusSearch(query, indices, dists, radius, flann::SearchParams());
}

bool mesh::prune_overhangs(float height, float step_height, float radius) {
    // TODO: parameterize this function with the up vector
    using namespace std;
    unordered_set<int> to_keep;

    for (int f : this->walkable_faces) {
        vector<vector<int>> indices; 
        vector<vector<float>> dists;
        vector<vector<float>> positions;

        aiVector3D f_center(0,0,0);
        this->get_face_center(f, f_center);

        int num_spheres = (int) round((height - step_height) / (2 * radius));

        for (int i = 0; i < num_spheres; ++i) {
            float y_val = f_center.y + step_height + radius + radius*i;

            positions.emplace_back(vector<float>({
                f_center.x, 
                y_val, 
                f_center.z
            }));
        }

        int faces_found = this->get_faces_in_radius(positions, radius, indices, dists);

        if (faces_found == 0)
            to_keep.insert(f);
    }

    this->clear_walkable_surfaces();
    this->walkable_faces = std::move(to_keep);

    return true;
}

bool mesh::prune_bottlenecks(float step_height, float radius) {
    // TODO: parameterize this function with the up vector
    using namespace std;
    unordered_set<int> to_keep;
    float fetch_radius = sqrt(
        pow(radius, 2) +
        pow(step_height, 2)
    );

    for (int f : this->walkable_faces) {
        vector<vector<int>> indices; 
        vector<vector<float>> dists;

        aiVector3D f_center(0,0,0);
        this->get_face_center(f, f_center);

        this->get_faces_in_radius(f, fetch_radius, indices, dists);

        float area = 0.0f;
        for (int n : indices[0]) {
            aiVector3D n_center(0.0f, 0.0f, 0.0f);
            this->get_face_center(n, n_center);

            // Check if this face is in the disk
            // check xz distance
            if(pow(n_center.x - f_center.x, 2) + pow(n_center.z - f_center.z, 2) > pow(radius, 2))
                continue;

            // check y distance
            if (abs(n_center.y - f_center.y) > step_height)
                continue;

            if (walkable_faces.count(n) > 0)
                area += this->get_face_area(n);
        }

        if (area >= 1.57079632679f*pow(radius,2)){
            to_keep.insert(f);

            for (int n : indices[0]) {
                aiVector3D n_center(0.0f, 0.0f, 0.0f);
                this->get_face_center(n, n_center);

                // Check if this face is in the disk
                // check xz distance
                if(pow(n_center.x - f_center.x, 2) + pow(n_center.z - f_center.z, 2) > pow(radius, 2))
                    continue;

                // check y distance
                if (abs(n_center.y - f_center.y) > step_height)
                    continue;

                if (walkable_faces.count(n) > 0)
                    to_keep.insert(n);
            }
        }

    }

    this->clear_walkable_surfaces();
    this->walkable_faces = std::move(to_keep);

    return true;
}

float mesh::get_face_area(int face) {
    using namespace std;
    aiMesh * mesh = scene->mMeshes[0];
    auto f = mesh->mFaces[face];

    
    if(f.mNumIndices < 3)
        return 0.0f;

    aiVector3D ab(
        mesh->mVertices[f.mIndices[1]].x - mesh->mVertices[f.mIndices[0]].x,
        mesh->mVertices[f.mIndices[1]].y - mesh->mVertices[f.mIndices[0]].y,
        mesh->mVertices[f.mIndices[1]].z - mesh->mVertices[f.mIndices[0]].z
    );
    
    aiVector3D ac(
        mesh->mVertices[f.mIndices[2]].x - mesh->mVertices[f.mIndices[0]].x,
        mesh->mVertices[f.mIndices[2]].y - mesh->mVertices[f.mIndices[0]].y,
        mesh->mVertices[f.mIndices[2]].z - mesh->mVertices[f.mIndices[0]].z
    );

    return 0.5 * sqrt(
        pow(ab.y * ac.z + ab.z * ac.y, 2) +
        pow(ab.z * ac.x + ab.x * ac.z, 2) +
        pow(ab.x * ac.y + ab.y * ac.x, 2)
    );
}
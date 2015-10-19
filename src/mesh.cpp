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

    if (scene->mNumMeshes != 1){
        std::cerr << "Scene contains does not contain exactly one mesh(" << scene->mNumMeshes << ")." << std::endl;
        return false;
    }

    return true;
}

bool mesh::export_to_file(const std::string& format, const std::string& filepath, const std::string & filename) {
    Assimp::Exporter exporter;

    // TODO: fix the memory leak from not deleting temp
    // Note that aiScene::~aiScene() is not exported by the library
    //      and as such fixing this leak is hard/impossible

    aiScene * temp;
    aiCopyScene(this->scene, &temp);
    keep_faces(this->walkable_faces, temp);

    if(AI_SUCCESS == exporter.Export(temp, format, filepath+filename)) {
        return true;
    }

    std::cerr << exporter.GetErrorString() << std::endl;
    return false;
}

void mesh::keep_faces(std::unordered_set<int> & to_keep, aiScene * s) {
    aiMesh * mesh = s->mMeshes[0];

    int num_new = 0;
    aiFace * new_faces = new aiFace[mesh->mNumFaces];

    // For each face in the mesh
    for (int i = 0; i < mesh->mNumFaces; ++i) {
        // If this face is in to_keep keep it
        if (to_keep.count(i) > 0) {
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

void mesh::walkable_by_gradient(float gradient) {
    // Vector representing the upwards direction
    aiVector3D n = aiVector3D(0.0f, 1.0f, 0.0f);
    aiMesh * mesh = scene->mMeshes[0];

    for (int i = 0; i < mesh->mNumFaces; ++i) {
        float angle = utils::angle_between(
            n, 
            get_face_normal(mesh->mFaces[i])
        );

        if (angle < gradient) {
            this->walkable_faces.insert(i);
        }
    }
}

void mesh::walkable_by_weighted_gradient(float radius, float gradient, int weighting) {
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

void mesh::connect_face_to_chunks(
    int f, 
    std::unordered_set<int> & to_add, 
    float step_distance, 
    std::unordered_set<int> & chunk
) {
    using namespace std;

    // If this polygon is surrounded by only polygons in the same chunk skip it
    bool surrounded = true;
    for (int n : neighbouring_triangles[f])
        surrounded &= !(chunk.count(n) == 0);

    if (surrounded)
        return;

    // Dijkstras
    unordered_map<int, float> dist;
    unordered_map<int, int> prev;
    priority_queue<S> pq;

    pq.push(S(f, 0.0f));

    while(!pq.empty()) {
        S min = pq.top();
        pq.pop();

        for (int n : neighbouring_triangles[min.face]) {
            S alt(n, distance(f, n));
            
            // Cuttoffs
            // Max step distance is exceeded
            if (alt.dist > step_distance)
                continue;

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

void mesh::graph_rejoin(float step_distance) {
    using namespace std;

    // setup all chunks
    this->setup_chunks();

    unordered_set<int> to_add;

    for (unordered_set<int> & c : this->chunks)
        for (int f : c)
            connect_face_to_chunks(f, to_add, step_distance, c);

    for (int f : to_add)
        walkable_faces.insert(f);

    // setup all chunks
    this->setup_chunks();
}

void mesh::keep_largest_region() {
    using namespace std;

    int size = 0;
    int index = -1;
    cout << "size of chunks: " << chunks.size() << endl;
    
    for (int i = 0; i < this->chunks.size(); ++i) {
        if (this->chunks[i].size() > size){
            size = this->chunks[i].size();
            index = i;
        }
    }

    for (int i = 0; i < chunks.size(); ++i){
        if (i != index)
            chunks[i].clear();
    }

    cout << "index: " << index << endl;
    cout << "size of chunks: " << chunks.size() << endl;
    this->walkable_faces = std::move(this->chunks[index]);
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

void mesh::remove_regions(int min_size) {
    using namespace std;

    aiMesh * mesh = scene->mMeshes[0];

    unordered_set<int> to_delete;

    for (unordered_set<int> & c : this->chunks)
        if (c.size() < min_size){
            for (int f : c)
                to_delete.insert(f);
            c.clear();
        }

    this->delete_faces(to_delete);
}

void mesh::remove_regions(float min_area) {
    using namespace std;

    unordered_set<int> to_delete;

    // For each chunk calculate its area
    for (unordered_set<int>  & c : this->chunks) {
        float chunk_area = 0.0f;

        for (int f : c)
            chunk_area += this->get_face_area(f);

        if (chunk_area < min_area){
            for (int f : c)
                to_delete.insert(f);
            c.clear();
        }
    }

    this->delete_faces(to_delete);
}

void mesh::clear_walkable_surfaces() {
    this->walkable_faces.clear();
}

void mesh::proximity_rejoin(float step_distance) {
    using namespace std;

    // setup all chunks
    this->setup_chunks();

    // setup a face to chunk mapping
    unordered_map<int,int> edge_to_chunk_map;
    for (int chunk = 0; chunk < chunks.size(); ++ chunk)
        for (int face : chunks[chunk])
            edge_to_chunk_map.emplace(face, chunk);

    // Map out all connected chunks
    vector<unordered_set<int>> connected_chunks(chunks.size(), unordered_set<int>());
    for (int chunk = 0; chunk < chunks.size(); ++ chunk) {

        // Find all chunks that are connected to the current chunk
        for (int face : chunks[chunk]){
            vector<vector<int>> indices; 
            vector<vector<float>> dists;

            this->get_faces_in_radius(face, step_distance, indices, dists);

            for (int cf : indices[0]){
                if (edge_to_chunk_map.count(cf) == 1)
                    if (edge_to_chunk_map[cf] != chunk)
                        connected_chunks[chunk].insert(edge_to_chunk_map[cf]);
            }
        }
    }

    // Flatten the connected chunk graph
    for (int chunk = 0; chunk < chunks.size(); ++ chunk) {
        queue<int> other_chunks;

        for (int c : connected_chunks[chunk])
            other_chunks.push(c);

        while(!other_chunks.empty()) {
            int oc = other_chunks.front();
            other_chunks.pop();

            // Add all element in this other chunk to the to_add queue if they are not already in the current face
            for (int x : connected_chunks[oc])
                if (connected_chunks[chunk].count(x) == 0){
                    connected_chunks[chunk].insert(x);
                    other_chunks.push(x);
                }

            connected_chunks[oc].clear();
        }
    }

    for (int chunk = 0; chunk < chunks.size(); ++ chunk) {
        for (int oc : connected_chunks[chunk]) {
            for (int f : chunks[oc])
                chunks[chunk].insert(f);
            chunks[oc].clear();
        }
    }
}

void mesh::setup_spatial_structures() {
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

void mesh::remove_overhangs(float height, float step_height, float radius) {
    using namespace std;
    unordered_set<int> to_keep;

    for (int f : this->walkable_faces) {
        vector<vector<int>> indices; 
        vector<vector<float>> dists;
        vector<vector<float>> positions;

        aiVector3D f_center(0,0,0);
        this->get_face_center(f, f_center);

        int num_spheres = (int) round((height - step_height - radius) / (radius));

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
}

void mesh::remove_bottlenecks(float step_height, float radius) {
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

void mesh::setup_chunks() {
    using namespace std;

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
        this->chunks.emplace_back(chunk);
    }
}
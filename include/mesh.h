#ifndef INCLUDE_MESH_H
#define INCLUDE_MESH_H

#include <assimp/scene.h>
#include <flann/flann.hpp>

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

class mesh {
    private:
        aiScene * scene;
        std::unordered_set<int> walkable_faces;
        flann::Index<flann::L2_Simple<float>> * f_index;
    
    public:
        std::unordered_map<int, std::unordered_set<int>> neighbouring_triangles;
        
        
        mesh();

        bool import_from_file(const std::string& filepath);
        bool export_to_file(const std::string& format, const std::string& filepath, const std::string & filename);

        bool is_walkable(int f);

        bool setup_neighbouring_triangles();
        void fill(
            int f,
            std::unordered_set<int> & faces,
            std::unordered_set<int> & chunk
        );
        bool cull_chunks(int min_size);



        bool rejoin_chunks(float distance);
        void better_spill(int f, std::unordered_set<int> & to_add, float distancee, std::unordered_set<int> & chunk);
        float distance(int face1, int face2);

        // make this method private as it is used only internally
        void keep_faces(std::unordered_set<int> & to_keep, aiScene * s);
        void delete_faces(std::unordered_set<int> & to_delete);
        bool keep_largest_chunk();

        void clear_walkable_surfaces();

        aiVector3D get_face_normal(const aiFace & face);
        // bool prune(float delta_angle);
        bool prune(float delta_angle, aiVector3D n = aiVector3D(0.0f, 1.0f, 0.0f));

        void setup_spatial_structures();

        void get_face_center(int face, aiVector3D & center);

        int get_faces_in_radius(int face, float radius, std::vector<std::vector<int>> & indices, std::vector<std::vector<float>> & dists);

        bool cull_chunks(float min_area);
        void merge_chunks();
        bool prune_overhangs(float height, float radius);
        bool prune_bottlenecks(float step_height, float radius);

        float get_face_area(int face);
};

#endif
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
        std::unordered_map<int, std::unordered_set<int>> neighbouring_triangles;
        std::vector<std::unordered_set<int>> chunks;
        
        void keep_faces(std::unordered_set<int> & to_keep, aiScene * s);
        void delete_faces(std::unordered_set<int> & to_delete);

        void fill(
            int f,
            std::unordered_set<int> & faces,
            std::unordered_set<int> & chunk
        );

        float get_face_area(int face);
        void get_face_center(int face, aiVector3D & center);
        float distance(int face1, int face2);
        aiVector3D get_face_normal(const aiFace & face);


        void setup_chunks();
        void connect_face_to_chunks(
            int f, 
            std::unordered_set<int> & to_add, 
            float step_distance, 
            std::unordered_set<int> & chunk
        );

        int get_faces_in_radius(int face, float radius, std::vector<std::vector<int>> & indices, std::vector<std::vector<float>> & dists);
        int get_faces_in_radius(const std::vector<std::vector<float>> & positions, float radius, std::vector<std::vector<int>> & indices, std::vector<std::vector<float>> & dists);

    public:
                
        mesh();
        // TODO: code a destructor to clean up all memory

        bool import_from_file(const std::string& filepath);
        bool export_to_file(const std::string& format, const std::string& filepath, const std::string & filename);

        void setup_neighbouring_triangles();
        void setup_spatial_structures();

        void walkable_by_gradient(float gradient);
        void walkable_by_weighted_gradient(float radius, float gradient, int weighting = 1);

        void remove_bottlenecks(float step_height, float radius, float min_area);
        void remove_overhangs(float height, float step_height, float radius);

        void graph_rejoin(float step_distance);
        void proximity_rejoin(float step_distance);

        void remove_regions(int min_size);
        void remove_regions(float min_area);
        void keep_largest_region();

        void clear_walkable_surfaces();
};

#endif
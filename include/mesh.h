#ifndef INCLUDE_MESH_H
#define INCLUDE_MESH_H

#include <assimp/scene.h>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

class mesh {
    private:
        aiScene * scene;
        std::unordered_set<int> walkable_faces;
    
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
        void spill(int origin_face, int current_face, std::unordered_set<int> & origin_chunk, float dist, std::vector<int> path, std::unordered_set<int> & visited, std::unordered_set<int> & to_add);
        void better_spill(int f, std::unordered_set<int> & to_add, float distancee, std::unordered_set<int> & chunk);
        float distance(int face1, int face2);

        // make this method private as it is used only internally
        void keep_faces(std::unordered_set<int> & to_keep, aiScene * s);
        void delete_faces(std::unordered_set<int> & to_delete);
        bool keep_largest_chunk();

        void clear_walkable_surfaces();

        aiVector3D get_face_normal(const aiFace & face);
        bool prune(float delta_angle);


        void color_faces();
};

#endif
#ifndef INCLUDE_MESH_H
#define INCLUDE_MESH_H

#include <assimp/scene.h>
#include <string>
#include <unordered_set>
#include <unordered_map>

class mesh {
    private:
        aiScene * scene;
    
    public:
        std::unordered_map<int, std::unordered_set<int>> neighbouring_triangles;
        
        mesh();

        bool import_from_file(const std::string& filepath);
        bool export_to_file(const std::string& format, const std::string& filepath);

        bool setup_neighbouring_triangles();
        void fill(
            int f,
            std::unordered_set<int> & faces,
            std::unordered_set<int> & chunk
        );
        bool cull_chunks(int min_size);

        void delete_faces(std::unordered_set<int> & to_delete);

        aiVector3D get_face_normal(const aiFace & face);
        bool prune();
};

#endif
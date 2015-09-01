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

        void setup_neighbouring_triangles();

        aiVector3D get_face_normal(const aiFace & face);
        bool prune();
};

#endif
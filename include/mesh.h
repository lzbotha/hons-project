#ifndef INCLUDE_MESH_H
#define INCLUDE_MESH_H

#include <assimp/scene.h>
#include <string>

class mesh {
    private:
        aiScene * scene;
    
    public:
        bool import_from_file(const std::string& filepath);
        bool export_to_file(const std::string& format, const std::string& filepath);

        bool prune();
};

#endif
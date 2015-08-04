#ifndef INCLUDE_UTILS_H
#define INCLUDE_UTILS_H

// TODO: find the smallest import that imports aiVector3D
#include <assimp/Importer.hpp>

namespace utils {

    float dot_product(const aiVector3D & a, const aiVector3D & b);
    float angle_between(const aiVector3D & a, const aiVector3D & b);
};

#endif
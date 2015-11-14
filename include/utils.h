#ifndef INCLUDE_UTILS_H
#define INCLUDE_UTILS_H

#include <assimp/Importer.hpp>

namespace utils {
    /**
    * Given 2 aiVector3D calculate their dot product.
    */
    float dot_product(const aiVector3D & a, const aiVector3D & b);

    /**
    * Given 2 vectors find the angle between them in radians.
    */
    float angle_between(const aiVector3D & a, const aiVector3D & b);
};

#endif
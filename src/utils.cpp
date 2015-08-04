#include <math.h>
#include "../include/utils.h"

float utils::dot_product(const aiVector3D & a, const aiVector3D & b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float utils::angle_between(const aiVector3D & a, const aiVector3D & b) {
    return acos(
        utils::dot_product(a, b) /
        (a.Length() * b.Length())
    );
}
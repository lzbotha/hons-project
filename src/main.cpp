#include <iostream>
#include <string>
#include <vector>

#include "../include/mesh.h"

void do_stuff(mesh & m, float angle, float dist, std::string filepath, std::string filename) {
    using namespace std;
    // cout << m.prune(angle) << endl;
    cout << m.prune_weighted_gradient(0.3f, 6*0.0872664626f) << endl;

    // m.prune_bottlenecks(0.3f, 0.5f);
    // m.prune_overhangs(20.0f, 0.5f, 0.05f);

    // cout << m.rejoin_chunks(dist) << endl;
    // cout << m.keep_largest_chunk() << endl;
    cout << m.export_to_file("obj", filepath, filename) << endl;
    m.clear_walkable_surfaces();
}

int main() {
    using namespace std;

    mesh m;
    cout << m.import_from_file("meshes/petra/Buildings/Wadi_Farasa/Whole_Area/Wadi_Model_GPS_with_constant.obj") << endl;
    // cout << m.import_from_file("meshes/overhang.obj") << endl;
    // cout << m.setup_neighbouring_triangles() << endl;
    m.setup_spatial_structures();
    // cout << m.prune(0.523599f) << endl;
    // m.prune_bottlenecks(0.4f, 0.8f);

    // cout << m.setup_neighbouring_triangles() << endl;
    float d5 = 0.0872664626f;
    float j01 = 0.1f;

    for (int j = 4; j <= 4; ++j){
        for (int a = 8; a <= 8; ++ a){
            string filename = "a"+ to_string(5*a) + "_j" + ("0-"+to_string(j)) + ".obj";
            do_stuff(m, d5*4, j01 * j, "meshes/euclidian_distance/", filename);
            // string filename = "test.obj";
            // do_stuff(m, d5*4, j01 * j, "meshes/", filename);
        }
    }

    return 0;
}
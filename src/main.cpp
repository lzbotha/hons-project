#include <iostream>
#include <string>
#include <vector>

#include "../include/mesh.h"


int main() {
    using namespace std;

    mesh m;
    // cout << m.import_from_file("meshes/petra/Buildings/Wadi_Farasa/Whole_Area/Wadi_Model_GPS_with_constant.obj") << endl;
    cout << m.import_from_file("meshes/test-area.obj") << endl;
    m.setup_neighbouring_triangles();
    m.setup_spatial_structures();

    // m.walkable_by_gradient(6*0.0872664626f);
    m.walkable_by_weighted_gradient(0.3f, 6*0.0872664626f, 30);

    // m.remove_bottlenecks(0.3f, 0.5f);
    m.remove_overhangs(6.0f, 0.3f, 0.1f);

    // m.proximity_rejoin(0.3f);
    // m.graph_rejoin(0.3f);

    // m.remove_regions(5);
    // m.remove_regions(20.0f);
    // m.keep_largest_region();

    cout << m.export_to_file("obj", "meshes/", "overhangs.obj") << endl;

    return 0;
}
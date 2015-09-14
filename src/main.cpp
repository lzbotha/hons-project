#include <iostream>

#include "../include/mesh.h"

int main() {
    using namespace std;

    mesh m;
    // cout << m.DoTheImportThing("meshes/ORD_JH_01.stl") << endl;
    cout << m.import_from_file("meshes/petra/Buildings/Wadi_Farasa/Whole_Area/Wadi_Model_GPS_with_constant.obj") << endl;
    cout << m.prune() << endl;
    cout << m.setup_neighbouring_triangles() << endl;
    cout << "rejoining chunks" << endl;
    cout << m.rejoin_chunks(1.0f) << endl;
    cout << "chunks rejoined" << endl;
    // cout << m.cull_chunks(100) << endl;
    cout << m.keep_largest_chunk() << endl;
    cout << m.export_to_file("obj" ,"meshes/wadi_farasa.obj") << endl;

    return 0;
}
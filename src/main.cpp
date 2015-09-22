#include <iostream>
#include <string>

#include "../include/mesh.h"

void do_stuff(mesh & m, float angle, float dist, std::string filename) {
    using namespace std;
    cout << m.prune(angle) << endl;
    cout << m.rejoin_chunks(dist) << endl;
    cout << m.keep_largest_chunk() << endl;
    cout << m.export_to_file("obj" ,filename) << endl;
    m.clear_walkable_surfaces();
}

int main() {
    using namespace std;

    mesh m;
    // cout << m.DoTheImportThing("meshes/ORD_JH_01.stl") << endl;
    cout << m.import_from_file("meshes/petra/Buildings/Wadi_Farasa/Whole_Area/Wadi_Model_GPS_with_constant.obj") << endl;
    cout << m.setup_neighbouring_triangles() << endl;
    

    do_stuff(m, 0.174532925f*4, 0.05f, "meshes/trials/40_0-05.obj");
    do_stuff(m, 0.174532925f*4, 0.10f, "meshes/trials/40_0-10.obj");
    do_stuff(m, 0.174532925f*4, 0.15f, "meshes/trials/40_0-15.obj");
    do_stuff(m, 0.174532925f*4, 0.20f, "meshes/trials/40_0-20.obj");

    do_stuff(m, 0.174532925f*4, 0.25f, "meshes/trials/40_0-25.obj");
    do_stuff(m, 0.174532925f*4, 0.35f, "meshes/trials/40_0-35.obj");
    do_stuff(m, 0.174532925f*4, 0.5f, "meshes/trials/40_0-5.obj");
    do_stuff(m, 0.174532925f*4, 0.6f, "meshes/trials/40_0-6.obj");
    do_stuff(m, 0.174532925f*4, 0.75f, "meshes/trials/40_0-75.obj");

    
    return 0;
}
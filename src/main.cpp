#include <iostream>

#include "../include/mesh.h"

int main() {
    using namespace std;

    mesh m;
    // cout << m.DoTheImportThing("meshes/ORD_JH_01.stl") << endl;
    cout << m.import_from_file("meshes/amphitheatre_old.obj") << endl;
    // cout << m.prune() << endl;
    m.setup_neighbouring_triangles();
    // cout << m.export_to_file("obj" ,"meshes/amphitheatre.obj") << endl;

    return 0;
}
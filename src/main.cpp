#include <iostream>

#include "../include/mesh.h"

int main() {
    using namespace std;

    mesh m;
    // cout << m.DoTheImportThing("meshes/ORD_JH_01.stl") << endl;
    cout << m.import_from_file("meshes/ORD_JH_01.stl") << endl;
    cout << m.prune() << endl;
    cout << m.export_to_file("stl" ,"meshes/02.stl") << endl;

    return 0;
}
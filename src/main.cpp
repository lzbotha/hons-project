#include <iostream>

#include "../include/mesh.h"

int main() {
    using namespace std;

    mesh m;
    // cout << m.DoTheImportThing("meshes/ORD_JH_01.stl") << endl;
    cout << m.import_from_file("meshes/kat1024_source.blend") << endl;
    m.export_to_file("stl" ,"meshes/01.stl");

    return 0;
}
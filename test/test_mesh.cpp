#include <iostream>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include "test_mesh.h"

void TestMesh::setUp() {
    using namespace std;

    m = new mesh();
    
}

void TestMesh::tearDown() {
    delete m;   
}

void TestMesh::testReadFile(){
    
}

void TestMesh::testWriteFile() {

}

void TestMesh::testPrune() {

}

void TestMesh::testMergeVerts() {

}


void TestMesh::testSetupNeighbouringTriangles() {
    using namespace std;
    m->import_from_file("meshes/tests/pyramid.obj");
    m->setup_neighbouring_triangles();

    for (int i = 0; i < m->neighbouring_triangles.size(); ++i) {
        cout << i;
        for (int t : m->neighbouring_triangles[i])
            cout << " " << t;
        cout << endl;
    }
}

void TestMesh::testCullChunks() {
    using namespace std;
    m->import_from_file("meshes/tests/pyramid.obj");
    m->setup_neighbouring_triangles();

    m->cull_chunks(5);       
}
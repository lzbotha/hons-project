#include <iostream>
#include <string>
#include <vector>

#include "../include/mesh.h"

void do_stuff(mesh & m, float angle, float dist, std::string filepath, std::string filename) {
    using namespace std;
    cout << m.prune(angle) << endl;
    cout << m.rejoin_chunks(dist) << endl;
    cout << m.keep_largest_chunk() << endl;
    cout << m.export_to_file("obj", filepath, filename) << endl;
    m.clear_walkable_surfaces();
}

int main() {
    using namespace std;

    // vector<string> filepaths = {
    //     "meshes/petra/Buildings/Wadi_Farasa/Wadi_Farasa_triclinium/",
    //     "meshes/petra/Buildings/Wadi_Farasa/Wadi_Farasa_part3/",
    //     "meshes/petra/Buildings/Wadi_Farasa/Wadi_Farasa_garden_tomb/",
    //     "meshes/petra/Buildings/Wadi_Farasa/Wadi_Farasa_part4/",
    //     "meshes/petra/Buildings/Wadi_Farasa/Wadi_Farasa_part2/",
    //     "meshes/petra/Buildings/Wadi_Farasa/Wadi_Farasa_soldiers_tomb/",
    //     "meshes/petra/Buildings/Wadi_Farasa/Wadi_Farasa_Unknowing_tomb/",
    //     "meshes/petra/Buildings/Wadi_Farasa/Wadi_Farasa_caverns/",
    //     "meshes/petra/Buildings/Wadi_Farasa/Wadi_Farasa_part1/",
    //     "meshes/petra/Buildings/Wadi_Farasa/Wadi_Farasa_recco_tomb/"
    // };

    // vector<string> filenames = {
    //     "Wadi_triclinium_tomb_colour_new.obj",
    //     "Wadi_part_3_colour.obj",
    //     "Wadi_garden_tomb_coloured_new.obj",
    //     "Wadi_part_4_colour.obj",
    //     "Wadi_part_2_colour.obj",
    //     "Wadi_soldiers_tomb_colour_new.obj",
    //     "Wadi_Unknowing_tomb_colour.obj",
    //     "Wadi_caverns_colour.obj",
    //     "Wadi_part_1_colour.obj",
    //     "Wadi_recco_tomb_colour_shift.obj"
    // };

    // for (int i = 0; i < filepaths.size(); ++i){
    //     mesh m;
    //     cout << m.import_from_file(filepaths[i]+filenames[i]) << endl;
    //     cout << m.setup_neighbouring_triangles() << endl;

    //     do_stuff(m, 0.174532925f*2, 0.5f, "meshes/wadi_farasa/", filenames[i]);
    // }

    // mesh m;

    cout << m.import_from_file("meshes/petra/Buildings/Wadi_Farasa/Whole_Area/Wadi_Model_GPS_with_constant.obj") << endl;
    cout << m.setup_neighbouring_triangles() << endl;
    do_stuff(m, 0.174532925f*3, 0.5f, "meshes/euclidian_distance/", "test.obj");
    

    return 0;
}
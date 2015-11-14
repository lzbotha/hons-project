#ifndef INCLUDE_MESH_H
#define INCLUDE_MESH_H

#include <assimp/scene.h>
#include <flann/flann.hpp>

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

class mesh {
    private:
        aiScene * scene;
        std::unordered_set<int> walkable_faces;
        flann::Index<flann::L2_Simple<float>> * f_index;
        std::unordered_map<int, std::unordered_set<int>> neighbouring_triangles;
        std::vector<std::unordered_set<int>> chunks;
        
        void keep_faces(std::unordered_set<int> & to_keep, aiScene * s);
        void delete_faces(std::unordered_set<int> & to_delete);

        void fill(
            int f,
            std::unordered_set<int> & faces,
            std::unordered_set<int> & chunk
        );

        float get_face_area(int face);
        void get_face_center(int face, aiVector3D & center);
        float distance(int face1, int face2);
        aiVector3D get_face_normal(const aiFace & face);


        void setup_chunks();
        void connect_face_to_chunks(
            int f, 
            std::unordered_set<int> & to_add, 
            float step_distance, 
            std::unordered_set<int> & chunk
        );

        int get_faces_in_radius(int face, float radius, std::vector<std::vector<int>> & indices, std::vector<std::vector<float>> & dists);
        int get_faces_in_radius(const std::vector<std::vector<float>> & positions, float radius, std::vector<std::vector<int>> & indices, std::vector<std::vector<float>> & dists);

    public:
                
        mesh();

        /**
        * Import a mesh file into the mesh object.
        * @param filepath the relative filepath to the mesh file.
        */
        bool import_from_file(const std::string& filepath);

        /**
        * Export the mesh contained by a mesh object to file.
        * @param format the file format to be used.
        * @param filepath the location to store the output mesh.
        * @param filename the name of the mesh file.
        */
        bool export_to_file(const std::string& format, const std::string& filepath, const std::string & filename);

        /**
        * Setup the map of neighbouring triangles based on inital mesh geometry. Triangles are
        * neighbours if they share a common edge.
        */
        void setup_neighbouring_triangles();

        /**
        * Inserts all triangles into the spatial index allowing spatial queries.
        */
        void setup_spatial_structures();

        /**
        * Removes all triangles from the mesh that have a gradient greater than provided.
        * @param gradient the maximum allowed gradient in radians
        */
        void walkable_by_gradient(float gradient);

        /**
        * Removes all triangles from the mesh that have a weighted gradient greater than the 
        * specified value.
        *
        * @param radius the distance around each triangle used to calculate the weighted average.
        * @param gradient the maximum allowed gradient.
        * @param weighting the weighting to give to the primary triangle in each calculation.
        */
        void walkable_by_weighted_gradient(float radius, float gradient, int weighting = 1);

        /**
        * Removes all triangles from the mesh that form part of a gap narrower than can be walked.
        * A triangle fails this test if it does not have sufficient walkable area within radius of it.
        *
        * @param step_height the size of a step in the given mesh file.
        * @param radius the radius for which to find walkable area.
        * @param the minmum walkable area required to consider a triangle walkable.
        */
        void remove_bottlenecks(float step_height, float radius, float min_area);

        /**
        * Removes all triangles from the mesh that have overhead triangles in a cylinder of 
        * radius wide and height tall.
        *
        * @param height The size of the overhead clearance required.
        * @param step_height The size of a step in the mesh.
        * @param radius The radius of the cylinder used to find overhead collisions.
        */
        void remove_overhangs(float height, float step_height, float radius);

        /**
        * Rejoins disjoint chunks of walkable triangles using a graph based approach.
        *
        * @param step_distance the maximum distance to be traversed along the graph to rejoin disjoint chunks.
        */
        void graph_rejoin(float step_distance);

        /**
        * Rejoins disjoint chunks of walkable triangles using a proximity based approach.
        *
        * @param step_distance the maximum distance that triangles are considered neighbours.
        */
        void proximity_rejoin(float step_distance);

        /**
        * Removes all walkable chunks from the mesh that consist of fewer than min_size triangles.
        * @param min_size the desired minimum triangle count of walkable chunks.
        */
        void remove_regions(int min_size);

        /**
        * Keeps all walkable connected chunks in the mesh that have at least min_area surface area.
        * @param min_area the minimum area of desired chunks.
        */
        void remove_regions(float min_area);

        /**
        * Keeps the largest connected chunk of walkable triangles in the mesh.
        */
        void keep_largest_region();

        /**
        * Removes all current walkable triangles from the mesh.
        */
        void clear_walkable_surfaces();
};

#endif
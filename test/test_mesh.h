#ifndef _MESH_TEST
#define _MESH_TEST

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "../include/mesh.h"

class TestMesh : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE(TestMesh);
    CPPUNIT_TEST(testReadFile);
    CPPUNIT_TEST(testWriteFile);
    CPPUNIT_TEST(testPrune);
    CPPUNIT_TEST(testMergeVerts);
    CPPUNIT_TEST(testSetupNeighbouringTriangles);
    CPPUNIT_TEST_SUITE_END();

    private:
        mesh * m;

    public:
        // Memory managemenet stuff
        void setUp();
        void tearDown();

        // Test cases
        void testReadFile();
        void testWriteFile();
        void testPrune();
        void testMergeVerts();
        void testSetupNeighbouringTriangles();
        
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestMesh );

#endif
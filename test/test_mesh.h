#ifndef _MESH_TEST
#define _MESH_TEST

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class TestMesh : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE(TestMesh);
    CPPUNIT_TEST(testPopulateFromSTL);
    CPPUNIT_TEST(testWriteSTL);
    CPPUNIT_TEST(testPrune);
    CPPUNIT_TEST(testMergeVerts);
    CPPUNIT_TEST_SUITE_END();

    private:


    public:
        // Memory managemenet stuff
        void setUp();
        void tearDown();

        // Test cases
        void testPopulateFromSTL();
        void testWriteSTL();
        void testPrune();
        void testMergeVerts();
        
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestMesh );

#endif
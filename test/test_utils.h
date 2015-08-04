#ifndef _UTILS_TEST
#define _UTILS_TEST

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class TestUtils : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE(TestUtils);
    CPPUNIT_TEST(testVectorDotProduct);
    CPPUNIT_TEST(testVectorMagnitude);
    CPPUNIT_TEST(testVectorNormalize);
    CPPUNIT_TEST(testVectorAngleBetween);
    CPPUNIT_TEST_SUITE_END();

    private:


    public:
        // Memory managemenet stuff
        void setUp();
        void tearDown();

        // Test cases
        void testVectorDotProduct();
        void testVectorMagnitude();
        void testVectorNormalize();
        void testVectorAngleBetween();
        
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestUtils );

#endif
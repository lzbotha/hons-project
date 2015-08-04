#include <iostream>

#include "test_utils.h"
#include "../include/utils.h"

void TestUtils::setUp() {
    using namespace std;
    
}

void TestUtils::tearDown() {
    
}

void TestUtils::testVectorDotProduct() {
    aiVector3D v1(1.0f, 2.0f, 3.0f);
    aiVector3D v2(3.0f, 2.0f, 1.0f);

    CPPUNIT_ASSERT_EQUAL(10.0f, utils::dot_product(v1, v2));
}

void TestUtils::testVectorMagnitude() {
    aiVector3D v1(1.0f, 0.0f, 0.0f);
    CPPUNIT_ASSERT_EQUAL(1.0f, v1.Length());

    aiVector3D v2(2.0f, 3.0f, 4.0f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        5.38516480713, 
        v2.Length(), 
        0.000001f
    );

    aiVector3D v3(2.57f, 3.34f, 4.12f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        5.89363215683, 
        v3.Length(), 
        0.000001f
    );
}

void TestUtils::testVectorNormalize() {
    aiVector3D v1(1.0f, 0.0f, 0.0f);
    v1.Normalize();
    CPPUNIT_ASSERT_EQUAL(
        1.0f,
        v1.Length()
    );

    aiVector3D v2(1.0f, 3.0f, 2.0f);
    v2.Normalize();
    CPPUNIT_ASSERT_DOUBLES_EQUAL (
        1.0f,
        v2.Length(),
        0.000001f
    );

    aiVector3D v3(1.123123f, 3.13131f, 0.5f);
    v3.Normalize();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        1.0f,
        v3.Length(),
        0.000001f
    );
}

void TestUtils::testVectorAngleBetween() {
    using namespace utils;

    aiVector3D v1(1.0f, 0.0f, 0.0f);
    aiVector3D v2(1.0f, 0.0f, 0.0f);
    CPPUNIT_ASSERT_EQUAL(0.0f, angle_between(v1, v2));

    aiVector3D v3(0.0f, 1.0f, 0.0f);
    aiVector3D v4(1.0f, 0.0f, 0.0f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        1.57079632679f, 
        angle_between(v3, v4),
        0.000001f
    );

    aiVector3D v5(1.0f, 0.0f, 0.0f);
    aiVector3D v6(1.0f, 1.0f, 0.0f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        1.57079632679f / 2, 
        angle_between(v5, v6),
        0.000001f
    );

    aiVector3D v7(3.0f, 0.0f, 0.0f);
    aiVector3D v8(5.0f, 5.0f, 0.0f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        1.57079632679f / 2, 
        angle_between(v7, v8),
        0.000001f
    );

    aiVector3D v9(3.0f, 4.0f, 0.0f);
    aiVector3D v10(-8.0f, 6.0f, 0.0f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        1.57079632679f, 
        angle_between(v9, v10),
        0.000001f
    );

    aiVector3D v11(4.0f, 0.0f, 7.0f);
    aiVector3D v12(-2.0f, 1.0f, 3.0f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        1.12521377f, 
        angle_between(v11, v12),
        0.0001f
    );
}
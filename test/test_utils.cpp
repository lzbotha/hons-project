#include <iostream>

#include "test_utils.h"

void TestUtils::setUp() {
    using namespace std;
    
}

void TestUtils::tearDown() {
    
}

void TestUtils::testVectorDotProduct() {
    // utils::vector v1(1.0f, 2.0f, 3.0f);
    // utils::vector v2(3.0f, 2.0f, 1.0f);

    // CPPUNIT_ASSERT_EQUAL(10.0f, utils::vector::dot(v1, v2));
}

void TestUtils::testVectorMagnitude() {
    // utils::vector v1(1.0f, 0.0f, 0.0f);
    // CPPUNIT_ASSERT_EQUAL(1.0f, utils::vector::mag(v1));

    // utils::vector v2(2.0f, 3.0f, 4.0f);
    // CPPUNIT_ASSERT_DOUBLES_EQUAL(
    //     5.38516480713, 
    //     utils::vector::mag(v2), 
    //     0.000001f
    // );

    // utils::vector v3(2.57f, 3.34f, 4.12f);
    // CPPUNIT_ASSERT_DOUBLES_EQUAL(
    //     5.89363215683, 
    //     utils::vector::mag(v3), 
    //     0.000001f
    // );
}

void TestUtils::testVectorNormalize() {
    // utils::vector v1(1.0f, 0.0f, 0.0f);
    // utils::vector::normalize(v1);
    // CPPUNIT_ASSERT_EQUAL(
    //     1.0f,
    //     utils::vector::mag(v1)
    // );

    // utils::vector v2(1.0f, 3.0f, 2.0f);
    // utils::vector::normalize(v2);
    // CPPUNIT_ASSERT_DOUBLES_EQUAL (
    //     1.0f,
    //     utils::vector::mag(v2),
    //     0.000001f
    // );

    // utils::vector v3(1.123123f, 3.13131f, 0.5f);
    // utils::vector::normalize(v3);
    // CPPUNIT_ASSERT_DOUBLES_EQUAL(
    //     1.0f,
    //     utils::vector::mag(v3),
    //     0.000001f
    // );
}

void TestUtils::testVectorAngleBetween() {
    // using namespace utils;

    // vector v1(1.0f, 0.0f, 0.0f);
    // vector v2(1.0f, 0.0f, 0.0f);
    // CPPUNIT_ASSERT_EQUAL(0.0f, vector::angle_between(v1, v2));

    // vector v3(0.0f, 1.0f, 0.0f);
    // vector v4(1.0f, 0.0f, 0.0f);
    // CPPUNIT_ASSERT_DOUBLES_EQUAL(
    //     1.57079632679f, 
    //     vector::angle_between(v3, v4),
    //     0.000001f
    // );

    // vector v5(1.0f, 0.0f, 0.0f);
    // vector v6(1.0f, 1.0f, 0.0f);
    // CPPUNIT_ASSERT_DOUBLES_EQUAL(
    //     1.57079632679f / 2, 
    //     vector::angle_between(v5, v6),
    //     0.000001f
    // );

    // vector v7(3.0f, 0.0f, 0.0f);
    // vector v8(5.0f, 5.0f, 0.0f);
    // CPPUNIT_ASSERT_DOUBLES_EQUAL(
    //     1.57079632679f / 2, 
    //     vector::angle_between(v7, v8),
    //     0.000001f
    // );

    // vector v9(3.0f, 4.0f, 0.0f);
    // vector v10(-8.0f, 6.0f, 0.0f);
    // CPPUNIT_ASSERT_DOUBLES_EQUAL(
    //     1.57079632679f, 
    //     vector::angle_between(v9, v10),
    //     0.000001f
    // );

    // vector v11(4.0f, 0.0f, 7.0f);
    // vector v12(-2.0f, 1.0f, 3.0f);
    // CPPUNIT_ASSERT_DOUBLES_EQUAL(
    //     1.12521377f, 
    //     vector::angle_between(v11, v12),
    //     0.0001f
    // );
}
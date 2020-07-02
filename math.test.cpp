#define CATCH_CONFIG_MAIN

#include "catch2/catch.hpp"
#include "simpleMath.h"

const vec3 v0{};
const vec3 v1{1.0f, 1.0f, 1.0f};

const mat4 m0; // all zeroes
const mat4 m1{
    {
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f}
    }
};
const mat4 m2{
    {
        {1.0f, 1.0f, 1.0f, 1.0f},
        {2.0f, 2.0f, 2.0f, 2.0f},
        {3.0f, 3.0f, 3.0f, 3.0f},
        {4.0f, 4.0f, 4.0f, 4.0f}
    }
};

TEST_CASE( "zero-vec * 1 = zero-vec" )
{
    REQUIRE( multiplyVec3(v0, 1.0f) == v0 );
}

TEST_CASE( "1-vec * 2 = 2-vec" )
{
    REQUIRE( multiplyVec3(v1, 2.0f) == vec3{2.0f, 2.0f, 2.0f} );
}

TEST_CASE( "0-matrix * 0-matrix = 0-matrix" )
{
    REQUIRE( multiplyMat4(m0, m0) == m0 );
    REQUIRE( multiplyMat4(m1, m0) == m0 );
    REQUIRE( multiplyMat4(m2, m0) == m0 );
}

TEST_CASE( "1-matrix * 1-matrix = 4-matrix" )
{
    REQUIRE( multiplyMat4(m1, m1) == mat4{
        {
            {4.0f, 4.0f, 4.0f, 4.0f},
            {4.0f, 4.0f, 4.0f, 4.0f},
            {4.0f, 4.0f, 4.0f, 4.0f},
            {4.0f, 4.0f, 4.0f, 4.0f}
        }
    } );
}

TEST_CASE( "1234-matrix * 1234-matrix = 10203040-matrix" )
{
    // printMat4(m2);
    // printMat4(multiplyMat4(m2, m2));
    REQUIRE( multiplyMat4(m2, m2) == mat4{
        {
            {10.0f, 10.0f, 10.0f, 10.0f},
            {20.0f, 20.0f, 20.0f, 20.0f},
            {30.0f, 30.0f, 30.0f, 30.0f},
            {40.0f, 40.0f, 40.0f, 40.0f}
        }
    } );
}

TEST_CASE( "1-matrix * 1234-matrix = 10-matrix" )
{
    // printMat4(multiplyMat4(m1, m2));
    REQUIRE( multiplyMat4(m1, m2) == mat4{
        {
            {10.0f, 10.0f, 10.0f, 10.0f},
            {10.0f, 10.0f, 10.0f, 10.0f},
            {10.0f, 10.0f, 10.0f, 10.0f},
            {10.0f, 10.0f, 10.0f, 10.0f}
        }
    } );
}

TEST_CASE( "1000-matrix * 0001-matrix both ways" )
{
    mat4 m1000;
    m1000.m[3][0] = 1.0f;

    mat4 m0001;
    m0001.m[0][3] = 1.0f;

    mat4 expected1;
    expected1.m[3][3] = 1.0f;

    mat4 expected2;
    expected2.m[0][0] = 1.0f;

    // printMat4(multiplyMat4(m1000, m0001));
    // printMat4(multiplyMat4(m0001, m1000));

    REQUIRE( multiplyMat4(m1000, m0001) == expected1);
    REQUIRE( multiplyMat4(m0001, m1000) == expected2);
}


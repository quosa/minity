#include <catch2/catch.hpp>
#include <iostream>

#include "simpleMath.h"

const vec3 v0{};
const vec3 v1{1.0f, 1.0f, 1.0f};

const mat4 m0; // all zeroes
const mat4 m1{
    {{1.0f, 1.0f, 1.0f, 1.0f},
     {1.0f, 1.0f, 1.0f, 1.0f},
     {1.0f, 1.0f, 1.0f, 1.0f},
     {1.0f, 1.0f, 1.0f, 1.0f}}};
const mat4 m2{
    {{1.0f, 1.0f, 1.0f, 1.0f},
     {2.0f, 2.0f, 2.0f, 2.0f},
     {3.0f, 3.0f, 3.0f, 3.0f},
     {4.0f, 4.0f, 4.0f, 4.0f}}};
const mat4 identity{
    {{1.0f, 0.0f, 0.0f, 0.0f},
     {0.0f, 1.0f, 0.0f, 0.0f},
     {0.0f, 0.0f, 1.0f, 0.0f},
     {0.0f, 0.0f, 0.0f, 1.0f}}};

TEST_CASE("zero-vec * 1 = zero-vec")
{
    REQUIRE(multiplyVec3(v0, 1.0f) == v0);
}

TEST_CASE("1-vec * 2 = 2-vec")
{
    REQUIRE(multiplyVec3(v1, 2.0f) == vec3{2.0f, 2.0f, 2.0f});
}

TEST_CASE("swap points (for sorting)")
{
    point a{1, 2, -3.0f};
    point b{4, 5, 6.0f};
    REQUIRE(a.x == 1);
    REQUIRE(b.y == 5);
    REQUIRE(a.z == -3.0f);
    pSwap(&a, &b);
    REQUIRE(b.x == 1);
    REQUIRE(a.y == 5);
    REQUIRE(b.z == -3.0f);
}

TEST_CASE("0-matrix * 0-matrix = 0-matrix")
{
    REQUIRE(multiplyMat4(m0, m0) == m0);
    REQUIRE(multiplyMat4(m1, m0) == m0);
    REQUIRE(multiplyMat4(m2, m0) == m0);
}

TEST_CASE("1-matrix * 1-matrix = 4-matrix")
{
    REQUIRE(multiplyMat4(m1, m1) == mat4{
                                        {{4.0f, 4.0f, 4.0f, 4.0f},
                                         {4.0f, 4.0f, 4.0f, 4.0f},
                                         {4.0f, 4.0f, 4.0f, 4.0f},
                                         {4.0f, 4.0f, 4.0f, 4.0f}}});
}

TEST_CASE("1234-matrix * 1234-matrix = 10203040-matrix")
{
    // printMat4(m2);
    // printMat4(multiplyMat4(m2, m2));
    REQUIRE(multiplyMat4(m2, m2) == mat4{
                                        {{10.0f, 10.0f, 10.0f, 10.0f},
                                         {20.0f, 20.0f, 20.0f, 20.0f},
                                         {30.0f, 30.0f, 30.0f, 30.0f},
                                         {40.0f, 40.0f, 40.0f, 40.0f}}});
}

TEST_CASE("1-matrix * 1234-matrix = 10-matrix")
{
    // printMat4(multiplyMat4(m1, m2));
    REQUIRE(multiplyMat4(m1, m2) == mat4{
                                        {{10.0f, 10.0f, 10.0f, 10.0f},
                                         {10.0f, 10.0f, 10.0f, 10.0f},
                                         {10.0f, 10.0f, 10.0f, 10.0f},
                                         {10.0f, 10.0f, 10.0f, 10.0f}}});
}

TEST_CASE("1000-matrix * 0001-matrix both ways")
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

    REQUIRE(multiplyMat4(m1000, m0001) == expected1);
    REQUIRE(multiplyMat4(m0001, m1000) == expected2);
}

TEST_CASE("matrix inverse - simple")
{
    // M * M^-1 = I
    const mat4 mt{
        {{1.0f, 1.0f, 1.0f, 10.0f},
         {-1.0f, 1.0f, 1.0f, 20.0f},
         {-1.0f, -1.0f, 1.0f, 30.0f},
         {0.0f, 0.0f, 0.0f, 1.0f}}};
    mat4 result;
    invertRowMajor((float *)mt.m, (float *)result.m);
    // printMat4(result);
    // std::cout << "---" << std::endl;
    // printMat4(multiplyMat4(result, mt));

    REQUIRE(multiplyMat4(result, mt) == identity);
}

TEST_CASE("matrix inverse for look-at camera")
{
    // M * M^-1 = I
    mat4 cameraMatrix = lookAtMatrixRH(vec3{4.0f, 3.0f, 3.0f}, vec3{0.0f, 0.0f, 0.0f}, vec3{0.0f, 1.0f, 0.0f});
    // std::cout << "CAMERA MATRIX:"  << std::endl;
    // printMat4(cameraMatrix);

    mat4 viewMatrix;
    invertRowMajor((float *)cameraMatrix.m, (float *)viewMatrix.m);
    // std::cout << "VIEW MATRIX:"  << std::endl;
    // printMat4(viewMatrix);

    // std::cout << "---" << std::endl;
    mat4 lookAtResult = multiplyMat4(viewMatrix, cameraMatrix);
    // printMat4(lookAtResult);
    // CLOSE ENOUGH...
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            lookAtResult.m[row][col] = round(1000 * lookAtResult.m[row][col]) / 1000.0;

    REQUIRE(lookAtResult == identity);
}

TEST_CASE("matrix inverse for fps camera")
{
    // M * M^-1 = I
    mat4 fpsCameraMatrix = fpsLookAtMatrixRH(vec3{4.0f, 3.0f, 3.0f}, 0.1f, -0.2f);
    // std::cout << "CAMERA MATRIX:"  << std::endl;
    // printMat4(fpsCameraMatrix);

    mat4 fpsViewMatrix;
    invertRowMajor((float *)fpsCameraMatrix.m, (float *)fpsViewMatrix.m);
    // std::cout << "VIEW MATRIX:"  << std::endl;
    // printMat4(fpsViewMatrix);

    // std::cout << "---" << std::endl;
    mat4 fpsResult = multiplyMat4(fpsViewMatrix, fpsCameraMatrix);
    // printMat4(fpsResult);
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            fpsResult.m[row][col] = round(1000 * fpsResult.m[row][col]) / 1000.0;

    // printMat4(fpsResult);
    REQUIRE(fpsResult == identity);
}

TEST_CASE("matrix rotation - X axis")
{
    // full cirle (2*PI) should land us right where we started
    const mat4 xrot = rotateXMatrix(deg2rad(360));

    // NOTE: this requires a bit of rounding!
    // See:
    //  - are_relatively_equal function in math lib
    //  - http://realtimecollisiondetection.net/blog/?p=89
    //  - https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
    //  - https://github.com/catchorg/Catch2/blob/devel/docs/assertions.md#floating-point-comparisons
    REQUIRE(multiplyVec3(v1, xrot) == v1);
}

TEST_CASE("matrix rotation - Y axis")
{
    // full cirle (2*PI) should land us right where we started
    const mat4 yrot = rotateYMatrix(deg2rad(360));

    // NOTE: this requires a bit of rounding!
    // See:
    //  - are_relatively_equal function in math lib
    //  - http://realtimecollisiondetection.net/blog/?p=89
    //  - https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
    //  - https://github.com/catchorg/Catch2/blob/devel/docs/assertions.md#floating-point-comparisons
    REQUIRE(multiplyVec3(v1, yrot) == v1);
}

TEST_CASE("matrix rotation - Z axis")
{
    // full cirle (2*PI) should land us right where we started
    const mat4 zrot = rotateZMatrix(deg2rad(360));

    // NOTE: this requires a bit of rounding!
    // See:
    //  - are_relatively_equal function in math lib
    //  - http://realtimecollisiondetection.net/blog/?p=89
    //  - https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
    //  - https://github.com/catchorg/Catch2/blob/devel/docs/assertions.md#floating-point-comparisons
    REQUIRE(multiplyVec3(v1, zrot) == v1);
}

TEST_CASE("scale matrix")
{
    const mat4 scale = scaleMatrix(1.0f, -2.0f, 0.0f);
    REQUIRE(multiplyVec3(v0, scale) == v0);
    REQUIRE(multiplyVec3(v1, scale) == vec3{1.0f, -2.0f, 0.0f});
}

TEST_CASE("translate matrix")
{
    const mat4 translate = translateMatrix(0.0f, -2.0f, 3.0f);
    REQUIRE(multiplyVec3(v0, translate) == vec3{0.0f, -2.0f, 3.0f});
    REQUIRE(multiplyVec3(v1, translate) == vec3{1.0f, -1.0f, 4.0f});
}

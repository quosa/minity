#include <catch2/catch.hpp>
#include <math.h>
#include <list>
#include <utility>
#include <iostream>
#include <stdexcept>

#include "simpleMath.h"


// for hiding std::cout
// auto old_buffer = std::cout.rdbuf(nullptr);
// auto old_buffer = std::cerr.rdbuf(nullptr);

const vec3 v0{};
const vec3 v1{1.0f, 1.0f, 1.0f};

const vec3 right{1.0f, 0.0f, 0.0f};
const vec3 left{-1.0f, 0.0f, 0.0f};
const vec3 up{0.0f, 1.0f, 0.0f};
const vec3 down{0.0f, -1.0f, 0.0f};
const vec3 forward{0.0f, 0.0f, 1.0f};
const vec3 back{0.0f, 0.0f, -1.0f};
const vec3 forwardleft = v3Normalize(v3Add(forward, left));
const vec3 forwardright = v3Normalize(v3Add(forward, right));
const vec3 backleft = v3Normalize(v3Add(back, left));
const vec3 backright = v3Normalize(v3Add(back, right));

// TODO: wait for c++20 and std::numbers::pi
const double pi = std::atan(1)*4;

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

TEST_CASE("vec3 multiplication by two")
{
    REQUIRE(v3Mul(v1, 2.0f) == vec3{2.0f, 2.0f, 2.0f});
}

TEST_CASE("vec3 multiplication by one")
{
    REQUIRE(v3Mul(v1, 1.0f) == v1);
}

TEST_CASE("dot products - orthogonal all quadrants")
{
    REQUIRE(v3DotProduct(up, right) == 0.0f);
    REQUIRE(v3DotProduct(right, down) == 0.0f);
    REQUIRE(v3DotProduct(down, left) == 0.0f);
    REQUIRE(v3DotProduct(left, up) == 0.0f);
    REQUIRE(v3DotProduct(right, forward) == 0.0f);
    REQUIRE(v3DotProduct(forward, left) == 0.0f);
    REQUIRE(v3DotProduct(left, back) == 0.0f);
    REQUIRE(v3DotProduct(back, right) == 0.0f);
}

TEST_CASE("dot products - parallel all quadrants")
{
    REQUIRE(v3DotProduct(right, right) == 1.0f);
    REQUIRE(v3DotProduct(left, left) == 1.0f);
    REQUIRE(v3DotProduct(up, up) == 1.0f);
    REQUIRE(v3DotProduct(down, down) == 1.0f);
    REQUIRE(v3DotProduct(forward, forward) == 1.0f);
    REQUIRE(v3DotProduct(back, back) == 1.0f);
}

TEST_CASE("dot products - opposites all axis")
{
    REQUIRE(v3DotProduct(left, right) == -1.0f);
    REQUIRE(v3DotProduct(up, down) == -1.0f);
    REQUIRE(v3DotProduct(forward, back) == -1.0f);
}

TEST_CASE("dot products - angles")
{
    REQUIRE(v3DotProduct(forward, right) == 0.0f);
    REQUIRE(v3DotProduct(forward, forward) == 1.0f);
    // v2 is so long that projection is full
    REQUIRE(v3DotProduct(forward, vec3{1.0f, 0.0f, 1.0f}) == 1.0f);
    // a**2 + b**2 == c**2 where c=1 and a=b (unit vector)
    // sin(45) = x / 1 = 0,707106781186548
    float unitSide = sinf(45*pi/180);
    REQUIRE(v3DotProduct(forward, v3Normalize(vec3{1.0f, 0.0f, 1.0f})) == unitSide); // 0.707106781186548f);
}

TEST_CASE("dot products - 45deg angles")
{
    // orthogonal, cos(90) == 0
    REQUIRE(v3DotProduct(forwardleft, forwardright) == 0.0f);

    // parallel, cos(0) == 1
    REQUIRE_THAT(
        v3DotProduct(forwardright, forwardright),
        Catch::Matchers::WithinAbs(1.0f, 0.000001)
    );
    // REQUIRE(v3DotProduct(forwardright, forwardright) == 1.0f);

    // opposites, cos(180) == -1
    REQUIRE_THAT(
        v3DotProduct(forwardright, backleft),
        Catch::Matchers::WithinAbs(-1.0f, 0.000001)
    );
    // REQUIRE(v3DotProduct(forwardright, backleft) == -1.0f);

    // orthogonal, cos(-90) == 0
    REQUIRE(v3DotProduct(forwardright, backright) == 0.0f);
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

TEST_CASE("transpose matrix")
{
    const mat4 matrix{
        {{1.1f, 1.2f, 1.3f, 1.4f},
         {2.1f, 2.2f, 2.3f, 2.4f},
         {3.1f, 3.2f, 3.3f, 3.4f},
         {4.1f, 4.2f, 4.3f, 4.4f}}};
    mat4 transpose;
    transpose = transposeMat4(matrix);
    // printMat4(transpose);
    REQUIRE(transposeMat4(transpose) == matrix);
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

TEST_CASE("inverse matrix")
{
    // const mat4 matrix{
    //     {{1.1f, 1.2f, 1.3f, 1.4f},
    //      {2.1f, 2.2f, 2.3f, 2.4f},
    //      {3.1f, 3.2f, 3.3f, 3.4f},
    //      {4.1f, 4.2f, 4.3f, 4.4f}}};
    const mat4 matrix{
        {{1.0f, 1.0f, 1.0f, 10.0f},
         {-1.0f, 1.0f, 1.0f, 20.0f},
         {-1.0f, -1.0f, 1.0f, 30.0f},
         {0.0f, 0.0f, 0.0f, 1.0f}}};
    mat4 inverse;
    inverse = invertMat4(matrix);
    // printMat4(inverse);
    REQUIRE(multiplyMat4(inverse, matrix) == identity);
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

// projectionMatrix(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
TEST_CASE("projection matrix - z at midpoint")
{
    const vec3 vInViewFrustum{1.0f, 1.0f, -2.0f};
    printVec3(vInViewFrustum);
    const mat4 projection = projectionMatrix(90.0f, 4.0f/3.0f, 1.0f, 3.0f);
    printMat4(projection);
    auto projected = multiplyVec3(vInViewFrustum, projection);
    printVec3(projected);
    auto vInClipSpace = v3Div(projected, projected.w);
    printVec3(vInClipSpace);

    REQUIRE(vInClipSpace.x == 0.75f/2.0f); // 1/ar == 0.75 as tan(45) = 1
    REQUIRE(vInClipSpace.y == 0.5f);
    REQUIRE(vInClipSpace.z == 0.5f); // already 80% of z-buffer used at midpoint
}

TEST_CASE("projection matrix - z at far plane")
{
    const vec3 vInViewFrustum{1.0f, 1.0f, -3.0f};
    printVec3(vInViewFrustum);
    const mat4 projection = projectionMatrix(90.0f, 4.0f/3.0f, 1.0f, 3.0f);
    printMat4(projection);
    auto projected = multiplyVec3(vInViewFrustum, projection);
    printVec3(projected);
    auto vInClipSpace = v3Div(projected, projected.w);
    printVec3(vInClipSpace);

    // x and y get smaller as teh point is farther away
    REQUIRE(vInClipSpace.x == 0.75f/3.0f); // 1/ar == 0.75 as tan(45) = 1
    REQUIRE(vInClipSpace.y == 1.0f/3.0f);
    REQUIRE(vInClipSpace.z == 1.0f);
}

TEST_CASE("projection matrix - max x and y values when z at near plane")
{
    const vec3 vInViewFrustum{4.0f/3.0f, 1.0f, -1.0f};
    printVec3(vInViewFrustum);
    const mat4 projection = projectionMatrix(90.0f, 4.0f/3.0f, 1.0f, 3.0f);
    printMat4(projection);
    auto projected = multiplyVec3(vInViewFrustum, projection);
    printVec3(projected);
    auto vInClipSpace = v3Div(projected, projected.w);
    printVec3(vInClipSpace);

    // x and y are at their max value at near plane
    REQUIRE(vInClipSpace.x == 1.0f);
    REQUIRE(vInClipSpace.y == 1.0f);
    REQUIRE(vInClipSpace.z == -1.0f);
}

TEST_CASE("projection matrix - center with z at near plane")
{
    const vec3 vInViewFrustum{0.0f, 0.0f, -1.0f};
    printVec3(vInViewFrustum);
    const mat4 projection = projectionMatrix(90.0f, 4.0f/3.0f, 1.0f, 3.0f);
    printMat4(projection);
    auto projected = multiplyVec3(vInViewFrustum, projection);
    printVec3(projected);
    auto vInClipSpace = v3Div(projected, projected.w);
    printVec3(vInClipSpace);

    REQUIRE(vInClipSpace.x == 0.0f);
    REQUIRE(vInClipSpace.y == 0.0f);
    REQUIRE(vInClipSpace.z == -1.0f);
}

TEST_CASE("projection matrix - min x and y values when z at near plane")
{
    const vec3 vInViewFrustum{-4.0f/3.0f, -1.0f, -1.0f};
    // printVec3(vInViewFrustum);
    const mat4 projection = projectionMatrix(90.0f, 4.0f/3.0f, 1.0f, 3.0f);
    // printMat4(projection);
    auto projected = multiplyVec3(vInViewFrustum, projection);
    // printVec3(projected);
    auto vInClipSpace = v3Div(projected, projected.w);
    // printVec3(vInClipSpace);

    // x and y are at their max value at near plane
    REQUIRE(vInClipSpace.x == -1.0f);
    REQUIRE(vInClipSpace.y == -1.0f);
    REQUIRE(vInClipSpace.z == -1.0f);
}

// mat4 lookAtMatrixRH(const vec3 &from, const vec3 &to, const vec3 &tmp = vec3{0.0f, 1.0f, 0.0f})
TEST_CASE("lookat camera matrix - from negative z")
{
    const vec3 v10{10.0f, 10.0f, 10.0f};
    const vec3 cameraPosition{0.0f, 0.0f, -10.0f};
    // camera at z=-10 looking towards origin and up vector is y-axis
    const mat4 lookat = lookAtMatrixRH(cameraPosition, v0, vec3{0.0f, 1.0f, 0.0f});

    // std::cout << multiplyVec3(v0, lookat).str() << std::endl;
    // std::cout << multiplyVec3(v1, lookat).str() << std::endl;
    // std::cout << multiplyVec3(v10, lookat).str() << std::endl;
    // std::cout << multiplyVec3(cameraPosition, lookat).str() << std::endl;

    REQUIRE(multiplyVec3(v0, lookat) == vec3{0.0f, 0.0f, -10.0f});
    REQUIRE(multiplyVec3(v1, lookat) == vec3{-1.0f, 1.0f, -11.0f});
    REQUIRE(multiplyVec3(v10, lookat) == vec3{-10.0f, 10.0f, -20.0f});
    REQUIRE(multiplyVec3(cameraPosition, lookat) == v0);
}

// mat4 lookAtMatrixRH(const vec3 &from, const vec3 &to, const vec3 &tmp = vec3{0.0f, 1.0f, 0.0f})
TEST_CASE("lookat camera matrix - from above")
{
    const vec3 v10{10.0f, 10.0f, 10.0f};
    // camera at y=10 looking towards origin and up vector is z-axis
    const mat4 lookat = lookAtMatrixRH(vec3{0.0f, 10.0f, 0.0f}, v0, vec3{0.0f, 0.0f, 1.0f});

    // std::cout << multiplyVec3(v0, lookat).str() << std::endl;
    // std::cout << multiplyVec3(v1, lookat).str() << std::endl;
    // std::cout << multiplyVec3(v10, lookat).str() << std::endl;

    REQUIRE(multiplyVec3(v0, lookat) == vec3{0.0f, 0.0f, -10.0f});
    REQUIRE(multiplyVec3(v1, lookat) == vec3{-1.0f, 1.0f, -9.0f});
    REQUIRE(multiplyVec3(v10, lookat) == vec3{-10.0f, 10.0f, 0.0f});
}

TEST_CASE("lookat camera matrix - from front above right")
{
    const vec3 v10{10.0f, 10.0f, 10.0f};
    // camera at y=10 looking towards origin and up vector is z-axis
    const mat4 lookat = lookAtMatrixRH(v10, v0, vec3{-10.0f, 10.0f, -10.0f});

    // std::cout << multiplyVec3(v0, lookat).str() << std::endl;
    // std::cout << multiplyVec3(v1, lookat).str() << std::endl;
    // std::cout << multiplyVec3(v10, lookat).str() << std::endl;

    REQUIRE(multiplyVec3(v0, lookat) == vec3{0.0f, 0.0f, -sqrtf(300.0f)}); // sqrt(x**2 + y**2 + z**2)
    REQUIRE(multiplyVec3(v1, lookat) == vec3{0.0f, 0.0f, -sqrtf(3*powf(9.0f,2.0f))}); // sqrt(x**2 + y**2 + z**2)
    REQUIRE(multiplyVec3(v10, lookat) == v0); // camera position is point position so center from camera povs
}

// mat4 lookAtMatrixRH(const vec3 &from, const vec3 &to, const vec3 &tmp = vec3{0.0f, 1.0f, 0.0f})
TEST_CASE("lookat camera matrix - points behind")
{
    // vp is behind the camera
    const vec3 vp{0.0f, 0.0f, -100.0f};
    // vp2 is behind and off-center (to right and below from camera)
    const vec3 vp2{-10.0f, -10.0f, -100.0f};
    // camera at z=-10 looking towards origin and up vector is y-axis
    const mat4 lookat = lookAtMatrixRH(vec3{0.0f, 0.0f, -10.0f}, v0, vec3{0.0f, 1.0f, 0.0f});

    // std::cout << multiplyVec3(vp, lookat).str() << std::endl;
    // std::cout << multiplyVec3(vp2, lookat).str() << std::endl;

    REQUIRE(multiplyVec3(vp, lookat) == vec3{0.0f, 0.0f, 90.0f});
    REQUIRE(multiplyVec3(vp2, lookat) == vec3{10.0f, -10.0f, 90.0f});
}

enum camera_type { kFPSCamera, kSimple };

struct pipeline_config
{
    float xScale = 1.0f;
    float yScale = 1.0f;
    float zScale = 1.0f;
    float xRotate = 0.0f;
    float yRotate = 0.0f;
    float zRotate = 0.0f;
    float xTranslate = 0.0f;
    float yTranslate = 0.0f;
    float zTranslate = 0.0f;

    vec3 cameraPosition {0.0f, 0.0f, 0.0f};
    // fps camera parameters
    float cameraXRotation = 0.0f;
    float cameraYRotation = 0.0f;
    // basic lookat camera parameters:
    vec3 cameraLookAt {0.0f, 0.0f, -10.0f};
    vec3 cameraUp {0.0f, 1.0f, 0.0f};
    camera_type cameraType = kFPSCamera;


    float fovDegrees = 90.0f;
    float aspectRatio = 4.0f/3.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    int screenWidth = 640;
    int screenHeight = 480;
};

bool pipeline(const pipeline_config &conf, const vec3 &modelVertex, point &onScreen) {
    // scale, rotate and translate matrices
    mat4 scaler = scaleMatrix(conf.xScale, conf.yScale, conf.zScale);
    mat4 xRotator = rotateXMatrix(conf.xRotate);
    mat4 yRotator = rotateYMatrix(conf.yRotate);
    mat4 zRotator = rotateZMatrix(conf.zRotate);
    mat4 translator = translateMatrix(conf.xTranslate, conf.yTranslate, conf.zTranslate);
    // order matters: scale > rotate > move (=translate)
    mat4 worldTransformations = multiplyMat4(xRotator, scaler);
    worldTransformations = multiplyMat4(yRotator, worldTransformations);
    worldTransformations = multiplyMat4(zRotator, worldTransformations);
    worldTransformations = multiplyMat4(translator, worldTransformations);

    // view matrix (camera)
    mat4 cameraMatrix;
    switch (conf.cameraType)
    {
    case kFPSCamera:
        cameraMatrix = fpsLookAtMatrixRH(conf.cameraPosition, conf.cameraXRotation, conf.cameraYRotation);
        std::cout << "using fps camera" << std::endl;
        break;
    case kSimple:
        cameraMatrix = lookAtMatrixRH(conf.cameraPosition, conf.cameraLookAt, conf.cameraUp);
        std::cout << "using simple camera" << std::endl;
        break;
    default:
        throw std::invalid_argument( "unknown camera type" );
        break;
    }

    mat4 viewMatrix = cameraMatrix;
    std::cout << "view:" << std::endl;
    printMat4(viewMatrix);
    // std::cout << "M*M-1" << std::endl;
    // printMat4(multiplyMat4(viewMatrix, cameraMatrix));


    // perspective projection
    mat4 projector = projectionMatrix(conf.fovDegrees, conf.aspectRatio, conf.nearPlane, conf.farPlane);


    // start in MODEL SPACE (local model coordinates from modeling software)

    // apply world transformations (scale, rotate, translate/move)
    vec3 worldVertex = multiplyVec3(modelVertex, worldTransformations);

    // in WORLD SPACE (common coordinates for all models in scene)

    // apply view matrix
    vec3 viewVertex = multiplyVec3(worldVertex, viewMatrix);

    // in VIEW SPACE (what coordinates the camera sees)

    // apply projection matrix
    vec3 clipVertex = multiplyVec3(viewVertex, projector);

    // in CLIP SPACE (homogeneous coordinates before w divide)
    // https://en.wikipedia.org/wiki/Homogeneous_coordinates

    // normalise into cartesian space
    vec3 ndcVertex = v3Div(clipVertex, clipVertex.w);

    // IN NDC SPACE (normalized device coordinates)

    bool clipped = !(ndcVertex.x >= -1 && ndcVertex.x <= 1
                    && ndcVertex.y >= -1 && ndcVertex.y <= 1
                    && ndcVertex.z >= -1 && ndcVertex.z <= 1);
    // copied from simpleGraphics.h screenXY() as this file tests maths
    onScreen = point{
        (int)(((ndcVertex.x + 1.0f) / 2.0f) * static_cast<float>(conf.screenWidth)),
        (int)((1.0f - ((ndcVertex.y + 1.0f) / 2.0f)) * static_cast<float>(conf.screenHeight)),
        (ndcVertex.z + 1.0f) / 2.0f};

    std::cout << "------ pipeline --------" << std::endl;
    std::cout << " model: "; printVec3(modelVertex);
    std::cout << " world: "; printVec3(worldVertex);
    std::cout << "  view: "; printVec3(viewVertex);
    std::cout << "  clip: "; printVec3(clipVertex);
    std::cout << "   ndc: "; printVec3(ndcVertex);
    if (clipped)
        std::cout << "VERTEX WILL BE CLIPPED! " << onScreen.str() <<std::endl;
    else
        std::cout << "screen: " << onScreen.str() << std::endl;
    std::cout << "------------------------" << std::endl;

    return true;
}

TEST_CASE("model>world>view>clip>screen space pipeline")
{
    point onScreen;
    pipeline_config conf;
    conf.xScale = 4.0f;
    conf.yScale = 4.0f;
    conf.zScale = 4.0f;

    // camera is at origin, looking towards the negative Z-axis
    SECTION("vertex in front of camera centered") {
        // center in front of camera 10 units away
        pipeline(conf, vec3{0.0f, 0.0f, -10.0f}, onScreen);
        REQUIRE(onScreen.x == 640/2); // center
        REQUIRE(onScreen.y == 480/2); // center
    }

    // screen coordinates (0,0) is top-left
    SECTION("vertex left top in front of camera") {
        // top left in front of camera 10 units away
        // note: aspect ratio 4/3 = 1.333... * 10 units
        pipeline(conf, vec3{-13.333333333f, 10.0f, -10.0f}, onScreen);
        REQUIRE(onScreen.x == 0); // left
        REQUIRE(onScreen.y == 0); // top
    }

    SECTION("vertex right top in front of camera") {
        // top right in front of camera 10 units away
        // note: aspect ratio 4/3 = 1.333... * 10 units
        pipeline(conf, vec3{13.333333333f, 10.0f, -10.0f}, onScreen);
        REQUIRE(onScreen.x == 640); // right
        REQUIRE(onScreen.y == 0); // top
    }

    SECTION("vertex left bottom in front of camera") {
        // top right in front of camera 10 units away
        // note: aspect ratio 4/3 = 1.333... * 10 units
        pipeline(conf, vec3{-13.333333333f, -10.0f, -10.0f}, onScreen);
        REQUIRE(onScreen.x == 0); // left
        REQUIRE(onScreen.y == 480); // bottom
    }

    SECTION("vertex right bottom in front of camera") {
        // top right in front of camera 10 units away
        // note: aspect ratio 4/3 = 1.333... * 10 units
        pipeline(conf, vec3{13.333333333f, -10.0f, -10.0f}, onScreen);
        REQUIRE(onScreen.x == 640); // right
        REQUIRE(onScreen.y == 480); // bottom
    }

    SECTION("vertex on far plane will be rendered") {
        // camera far plane is 100
        // translation scales by 4
        pipeline(conf, vec3{0.0f, 0.0f, -25.0f}, onScreen);
        REQUIRE(onScreen.z == 1.0f); // inside NDC cube xyz in [-1,1]
    }

    SECTION("vertex too far will be clipped") {
        // camera far plane is 100 (100.1/4=25.025)
        pipeline(conf, vec3{1.234f, -5.678f, -25.025f}, onScreen);
        REQUIRE(onScreen.z > 1.0f); // outside NDC cube (behind), will be clipped
    }

    SECTION("vertex too near will be clipped") {
        // camera near plane is 0.1, vertex centered to keep x, y inside ndc
        // translation scales by 4 (0.09/4=0.0225)
        pipeline(conf, vec3{0.0f, 0.0f, -0.0225f}, onScreen);
        // we selected screen z buffer to go from [0,1] so test for under zero here:
        REQUIRE(onScreen.z < 0.0f); // outside NDC cube (in front), will be clipped
    }

    SECTION("vertex on near plane will be rendered") {
        // camera near plane is 0.1, vertex centered to keep x, y inside ndc
        // translation scales by 4 (0.1/4=0.025)
        pipeline(conf, vec3{0.0f, 0.0f, -0.025f}, onScreen);
        // we selected screen z buffer to go from [0,1] so test for zero here:
        REQUIRE(onScreen.z == 0.0f); // inside NDC cube xyz in [-1,1]

    }
}

// TEST_CASE("pipeline with off-center camera"4)
// {
//     point onScreen;
//     pipeline_config conf;
//     // place came off-center: 1 right, 2 high, 3 far
//     conf.cameraPosition = vec3 {1.0f, 1.0f, 0.0f};
//     conf.cameraLookAt = vec3 {0.0f, 0.0f, -1.0f}; // through world origin
//     conf.cameraUp = v3Normalize(vec3 {-1.0f, 1.0f, -1.0f});
//
//     pipeline(conf, vec3{0.0f, 0.0f, -1.0f}, onScreen);
//     REQUIRE(onScreen.x == 640/2); // center
//     REQUIRE(onScreen.y == 480/2); // center
// }

TEST_CASE("pipeline with off-center camera high")
{
    point onScreen;
    pipeline_config conf;
    conf.cameraPosition = vec3 {0.0f, 1.0f, 0.0f};
    conf.cameraLookAt = vec3 {0.0f, 0.0f, -10.0f}; // through world origin
    conf.cameraUp = vec3 {0.0f, 1.0f, 0.0f};
    conf.cameraType = kSimple;

    pipeline(conf, vec3{0.0f, 0.0f, -10.0f}, onScreen);
    REQUIRE(onScreen.x == 640/2); // center
    REQUIRE(onScreen.y >= 480/2 - 1); // ~center with rounding error :-/
    REQUIRE(onScreen.y <= 480/2 + 1); // ~center with rounding error :-/
}

TEST_CASE("pipeline with off-center camera low")
{
    point onScreen;
    pipeline_config conf;
    conf.cameraPosition = vec3 {0.0f, -1.0f, 0.0f};
    conf.cameraLookAt = vec3 {0.0f, 0.0f, -10.0f}; // through world origin
    conf.cameraUp = vec3 {0.0f, 1.0f, 0.0f};
    conf.cameraType = kSimple;

    pipeline(conf, vec3{0.0f, 0.0f, -10.0f}, onScreen);
    REQUIRE(onScreen.x == 640/2); // center
    REQUIRE(onScreen.y >= 480/2 - 1); // ~center with rounding error :-/
    REQUIRE(onScreen.y <= 480/2 + 1); // ~center with rounding error :-/
}

TEST_CASE("pipeline with off-center camera right")
{
    point onScreen;
    pipeline_config conf;
    // simple
    conf.cameraPosition = vec3 {1.0f, 0.0f, 1.0f};
    conf.cameraLookAt = vec3 {0.0f, 0.0f, 0.0f};
    conf.cameraUp = vec3 {0.0f, 1.0f, 0.0f};
    // fps
    conf.cameraXRotation = 0.0f;
    conf.cameraYRotation = deg2rad(45.0f);
    conf.cameraType = kSimple; // kFPSCamera;

    // pipeline(conf, vec3{0.0f, 0.0f, -10.0f}, onScreen);
    pipeline(conf, vec3{-1.0f, 0.0f, -1.0f}, onScreen);
    REQUIRE(onScreen.x == 640/2); // center
    REQUIRE(onScreen.y >= 480/2 - 1); // ~center with rounding error :-/
    REQUIRE(onScreen.y <= 480/2 + 1); // ~center with rounding error :-/
}

TEST_CASE("lookat camera matrix - simple")
{
    // world point
    const vec3 wp{-1.0f, 0.0f, -1.0f};
    // camera at z=-10 looking towards origin and up vector is y-axis
    const mat4 lookat = lookAtMatrixRH(vec3{1.0f, 0.0f, 1.0f}, v0, vec3{0.0f, 1.0f, 0.0f});

    // std::cout << multiplyVec3(wp, lookat).str() << std::endl;

    vec3 result = multiplyVec3(wp, lookat);
    REQUIRE( result.x == 0.0f );
    REQUIRE( result.y == 0.0f );
    // world coordinates -2x, 0y, -2z -> sqrt(2*2**2) hypotenuse
    REQUIRE_THAT( result.z, Catch::Matchers::WithinRel(-sqrtf(8.0f), 0.0000001f) );
}

TEST_CASE("lookat camera matrix - multiple points")
{
    vec3 eye{1, 0, 1};
    vec3 tmp{0,1,0};
    vec3 center{0, 0, 0};
    const mat4 lookat = lookAtMatrixRH(eye, center, tmp);

    std::list<std::pair<vec3,vec3>> testCases {
        {{-1, 0, -1},           {        0, 0, -sqrtf(8.0f)}}, // -Z' (-2, 0, -2 in world-coordinates)
        {{ 1, 0, -1},           { sqrtf(2.0f), 0, -sqrtf(2.0f)}}, // X', -Z' (0, 0, -2 in world-coordinates)
        {{-1, 0,  1},           {-sqrtf(2), 0, -sqrtf(2.0f)}}, // -X', -Z' (-2, 0, 0 in world-coordinates)
        {{ 0, 0,  0},           {0,0,-sqrtf(2.0f)}}, // -Z' (-1, 0, -1 in world-coordinates)
    };

    for (auto p: testCases)
    {
        std::cout << "case: " << p.first << " -> " << p.second << std::endl;
        vec3 wp = p.first;
        vec3 expected = p.second;
        vec3 result = multiplyVec3(wp, lookat);
        // std::cout << "wp " << wp << " -> " << result << std::endl;

        REQUIRE_THAT( result.x, Catch::Matchers::WithinRel(expected.x, 0.0000001f) );
        REQUIRE_THAT( result.y, Catch::Matchers::WithinRel(expected.y, 0.0000001f) );
        REQUIRE_THAT( result.z, Catch::Matchers::WithinRel(expected.z, 0.0000001f) );

    };
}


TEST_CASE("lookat camera matrix - double-check with components")
{
    // vec3 wp{-1, 0, -1};
    vec3 eye{1, 0, 1};

    mat4 translation {
        {
            { 1, 0, 0, -eye.x },
            { 0, 1, 0, -eye.y },
            { 0, 0, 1, -eye.z },
            { 0, 0, 0,      1 },
        }
    };

    // std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    // std::cout << "trl" << std::endl; printMat4(translation);
    // std::cout << "wp       "; printVec3(wp);
    // std::cout << "wp * trl " << multiplyVec3(wp, translation) << std::endl;


    // Rotation part by intuition (vectors)
    mat4 xrot = rotateXMatrix(deg2rad(0));
    // std::cout << "xrot " << std::endl; printMat4(xrot);
    mat4 yrot = rotateYMatrix(deg2rad(-45));
    // std::cout << "yrot " << std::endl; printMat4(yrot);
    mat4 zrot = rotateZMatrix(deg2rad(0));
    // std::cout << "zrot " << std::endl; printMat4(zrot);
    mat4 xyzrot = multiplyMat4(zrot, multiplyMat4(yrot, xrot));
    // std::cout << "xyzrot (from components)" << std::endl; printMat4(xyzrot);
    // std::cout << "wp            "; printVec3(wp);
    // std::cout << "wp * xyzrot = " << multiplyVec3(wp, xyzrot) << std::endl;

    // mat4 xyzlookat = multiplyMat4(translation, xyzrot);
    mat4 xyzlookat = multiplyMat4(xyzrot, translation);

    // std::cout << "final xyzlookat (translation * xyzrotation)" << std::endl; printMat4(xyzlookat);
    // std::cout << "wp        "; printVec3(wp);
    // std::cout << "wp * xyzlookat = " << multiplyVec3(wp, xyzlookat) << std::endl;

    std::list<std::pair<vec3,vec3>> testCases {
        {{-1, 0, -1},           {        0, 0, -sqrtf(8.0f)}}, // -Z' (-2, 0, -2 in world-coordinates)
        {{ 1, 0, -1},           { sqrtf(2.0f), 0, -sqrtf(2.0f)}}, // X', -Z' (0, 0, -2 in world-coordinates)
        {{-1, 0,  1},           {-sqrtf(2), 0, -sqrtf(2.0f)}}, // -X', -Z' (-2, 0, 0 in world-coordinates)
        {{ 0, 0,  0},           {0,0,-sqrtf(2.0f)}}, // -Z' (-1, 0, -1 in world-coordinates)
    };

    for (auto p: testCases)
    {
        std::cout << "case: " << p.first << " -> " << p.second << std::endl;
        vec3 wp = p.first;
        vec3 expected = p.second;
        vec3 result = multiplyVec3(wp, xyzlookat);
        // std::cout << "wp " << wp << " -> " << result << std::endl;

        REQUIRE_THAT( result.x, Catch::Matchers::WithinRel(expected.x, 0.0000001f) );
        REQUIRE_THAT( result.y, Catch::Matchers::WithinRel(expected.y, 0.0000001f) );
        REQUIRE_THAT( result.z, Catch::Matchers::WithinRel(expected.z, 0.0000001f) );
    };
}

TEST_CASE("calculate face normal - facing right")
{
    // clockwise winding order
    // i.e. left-hand rule (thumb faces front)
    // (center > up > back faces right)
    vec3 faceNormal = v3Normalize(v3CrossProduct(
        v3Sub(back, v0), // v2 - v0
        v3Sub(up, v0) // v1 - v0
    ));
    REQUIRE(faceNormal == right);
}

TEST_CASE("calculate face normal - facing left")
{
    // clockwise winding order
    // i.e. left-hand rule (thumb faces front)
    // (center > back > up faces left)
    vec3 faceNormal = v3Normalize(v3CrossProduct(
        v3Sub(up, v0), // v2 - v0
        v3Sub(back, v0) // v1 - v0
    ));
    REQUIRE(faceNormal == left);
}

TEST_CASE("calculate face normal - facing forward")
{
    // clockwise winding order
    // i.e. left-hand rule (thumb faces front)
    // (center > up > right faces front)
    vec3 faceNormal = v3Normalize(v3CrossProduct(
        v3Sub(right, v0), // v2 - v0
        v3Sub(up, v0) // v1 - v0
    ));
    REQUIRE(faceNormal == forward);
}

TEST_CASE("calculate face normal - facing backward")
{
    // clockwise winding order
    // i.e. left-hand rule (thumb faces front)
    // (center > up > left faces back)
    vec3 faceNormal = v3Normalize(v3CrossProduct(
        v3Sub(left, v0), // v2 - v0
        v3Sub(up, v0) // v1 - v0
    ));
    REQUIRE(faceNormal == back);
}

TEST_CASE("calculate face normal - facing up")
{
    // clockwise winding order
    // i.e. left-hand rule (thumb faces front)
    // (center > back > right faces up)
    vec3 faceNormal = v3Normalize(v3CrossProduct(
        v3Sub(right, v0), // v2 - v0
        v3Sub(back, v0) // v1 - v0
    ));
    REQUIRE(faceNormal == up);
}

TEST_CASE("calculate face normal - facing down")
{
    // clockwise winding order
    // i.e. left-hand rule (thumb faces front)
    // (center > back > left faces down)
    vec3 faceNormal = v3Normalize(v3CrossProduct(
        v3Sub(left, v0), // v2 - v0
        v3Sub(back, v0) // v1 - v0
    ));
    REQUIRE(faceNormal == down);
}

TEST_CASE("adjust color by one")
{
    u_int32_t color{0x11223344};
    REQUIRE(adjustColor(color, 1.0f) == color);
}

TEST_CASE("adjust color by zero")
{
    u_int32_t color{0x11223344};
    REQUIRE(adjustColor(color, 0.0f) == 0x00000044); // alpha is not touched
}

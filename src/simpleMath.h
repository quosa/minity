#pragma once

#include <cmath>
#include <string>
#include <vector>
#include <iostream> // << overload for catch2


// forward declarations
float are_relatively_equal(const float a, const float b);

struct point
{
    int x = 0;      // top
    int y = 0;      // left
    float z = 0.0f; // z depth;
    std::string str()
    {
        return "("
            + std::to_string(this->x)
            + ", "
            + std::to_string(this->y)
            + ", "
            + std::to_string(this->z)
            + ")";
    };
};
struct vec3
{
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 1;
    // structs don't have default comparison
    bool operator==(const vec3 &other) const
    {
        return are_relatively_equal(this->x, other.x)
            && are_relatively_equal(this->y, other.y)
            && are_relatively_equal(this->z, other.z)
            && are_relatively_equal(this->w, other.w);
    };
    bool operator!=(const vec3 &other) const
    {
        return !(*this == other);
    };
    std::string str()
    {
        return "("
            + std::to_string(this->x)
            + ", " + std::to_string(this->y)
            + ", " + std::to_string(this->z)
            + ")";
    };
    friend std::ostream& operator<<(std::ostream& os, const vec3 &value);
};

struct tri
{
    vec3 vertices[3];
    u_int32_t color = 0xffffffff;
};

struct mesh
{
    bool enabled = true;
    std::vector<tri> tris;
    vec3 scale{1.0f, 1.0f, 1.0f};
    vec3 rotation{};
    vec3 translation{};
};

struct mat4
{
    float m[4][4] = {{0}};
    bool operator==(const mat4 &other) const
    {
        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
            {
                if (!are_relatively_equal(this->m[r][c], other.m[r][c]))
                    return false;
            }
        }
        return true;
    };
    friend std::ostream& operator<<(std::ostream& os, const mat4 &value);
};

// forward declarations:
void printMat4(const mat4 &mat, std::ostream& os = std::cout);
void printVec3(const vec3 &v);
void printTri(const tri &t, std::string label);


#ifndef MATH_TYPES_ONLY

// mainly for catch2 to be able to print the assertions
std::ostream& operator<<( std::ostream &os, const vec3 &value )
{
    os << "("
            + std::to_string(value.x)
            + ", " + std::to_string(value.y)
            + ", " + std::to_string(value.z)
            + ")";
    return os;
}


std::ostream& operator<<(std::ostream& os, const mat4 &value)
{
    printMat4(value, os);
    return os;
}

// adapted from http://realtimecollisiondetection.net/blog/?p=89
// todo: consider an ulp based solution as explained here:
// https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
float are_relatively_equal(const float a, const float b)
{
    // std::cout << std::fabs(a - b) << " <= " << std::numeric_limits<float>::epsilon() * std::max(std::fabs(a), std::fabs(b)) << std::endl;
    // 2 multiplier is selected to get the 360/2*pi rotations to work
    return (std::fabs(a - b) <= 2 * std::numeric_limits<float>::epsilon() * std::max(1.0f, std::max(std::fabs(a), std::fabs(b))));
};

constexpr float deg2rad(float degrees)
{
    return degrees * M_PI / 180.0f;
}

constexpr float rad2deg(float radians)
{
    return radians * 180.0f / M_PI;
}

// TODO: use template
void pSwap(point *a, point *b)
{
    point tmp = *a;
    *a = *b;
    *b = tmp;
}

vec3 v3Add(const vec3 &v1, const vec3 &v2)
{
    return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

vec3 v3Sub(const vec3 &v1, const vec3 &v2)
{
    return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

vec3 v3Mul(const vec3 &v1, const float k)
{
    return {v1.x * k, v1.y * k, v1.z * k};
}

vec3 v3Div(const vec3 &v1, const float k)
{
    return {v1.x / k, v1.y / k, v1.z / k};
}

float v3DotProduct(const vec3 &v1, const vec3 &v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

vec3 v3CrossProduct(const vec3 &v1, const vec3 &v2)
{
    vec3 out;
    out.x = v1.y * v2.z - v1.z * v2.y;
    out.y = v1.z * v2.x - v1.x * v2.z;
    out.z = v1.x * v2.y - v1.y * v2.x;
    return out;
}

float v3Length(const vec3 &v)
{
    return sqrtf(v3DotProduct(v, v));
}

vec3 v3Normalize(const vec3 &v)
{
    float l = v3Length(v);
    return {v.x / l, v.y / l, v.z / l};
}

mat4 scaleMatrix(const float x, const float y, const float z)
{
    mat4 out;
    out.m[0][0] = x;
    out.m[1][1] = y;
    out.m[2][2] = z;
    out.m[3][3] = 1;
    return out;
};

mat4 rotateXMatrix(const float fAngleRad)
{
    mat4 out;
    // row order: matrix[row][col]
    out.m[0][0] = 1.0f;
    out.m[1][1] = cosf(fAngleRad);
    out.m[1][2] = -sinf(fAngleRad);
    out.m[2][1] = sinf(fAngleRad);
    out.m[2][2] = cosf(fAngleRad);
    out.m[3][3] = 1.0f;
    return out;
}

mat4 rotateYMatrix(const float fAngleRad)
{
    mat4 out;
    // row order: matrix[row][col]
    out.m[0][0] = cosf(fAngleRad);
    out.m[0][2] = sinf(fAngleRad);
    out.m[1][1] = 1.0f;
    out.m[2][0] = -sinf(fAngleRad);
    out.m[2][2] = cosf(fAngleRad);
    out.m[3][3] = 1.0f;
    return out;
}

mat4 rotateZMatrix(float fAngleRad)
{
    mat4 out;
    // row order: matrix[row][col]
    out.m[0][0] = cosf(fAngleRad);
    out.m[0][1] = -sinf(fAngleRad);
    out.m[1][0] = sinf(fAngleRad);
    out.m[1][1] = cosf(fAngleRad);
    out.m[2][2] = 1.0f;
    out.m[3][3] = 1.0f;
    return out;
}

mat4 translateMatrix(const float x, const float y, const float z)
{
    mat4 out;
    // row order: matrix[row][col]
    out.m[0][0] = 1;
    out.m[1][1] = 1;
    out.m[2][2] = 1;
    out.m[3][3] = 1;
    out.m[0][3] = x;
    out.m[1][3] = y;
    out.m[2][3] = z;
    return out;
};

// perspective projection:
// see: https://unspecified.wordpress.com/2012/06/21/calculating-the-gluperspective-matrix-and-other-opengl-matrix-maths/
mat4 projectionMatrix(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
{
    assert(fNear >= 0.0f && fFar >= 0.0f && fNear <= fFar);

    float fFovRad = tanf(deg2rad(fFovDegrees) * 0.5f);

    mat4 out;
    // row order: matrix[row][col]
    // x
    out.m[0][0] = 1 / (fAspectRatio * fFovRad);
    // y
    out.m[1][1] = 1 / fFovRad;
    // z
    // There is also a weirdness because OpenGL is supposed to use a right-handed coordinate system:
    // the Z axis is inverted (recall that the -Z axis points away from the camera,
    // but in the depth buffer, smaller z values are considered closer).
    // To fix this, we can invert the Z coordinate (...[2][2] = -1)
    out.m[2][2] = (fFar + fNear) / (fNear - fFar);      // Same as OpenGL: - (zFar + zNear) / (zFar - zNear);
    out.m[2][3] = (2 * fFar * fNear) / (fNear - fFar);  // Same as OpenGL: - (2 * zFar * zNear) / (zFar - zNear);
    // w
    out.m[3][2] = -1.0f; // w = -z which will be divided again
    return out;
};

vec3 multiplyVec3(const vec3 &i, const float f)
{
    vec3 out = i;
    out.x *= f;
    out.y *= f;
    out.z *= f;
    return out;
};

vec3 multiplyVec3(const vec3 &i, const mat4 &m)
{
    // row order: matrix[row][col]
    vec3 o;
    o.x = i.x * m.m[0][0] + i.y * m.m[0][1] + i.z * m.m[0][2] + i.w * m.m[0][3];
    o.y = i.x * m.m[1][0] + i.y * m.m[1][1] + i.z * m.m[1][2] + i.w * m.m[1][3];
    o.z = i.x * m.m[2][0] + i.y * m.m[2][1] + i.z * m.m[2][2] + i.w * m.m[2][3];
    o.w = i.x * m.m[3][0] + i.y * m.m[3][1] + i.z * m.m[3][2] + i.w * m.m[3][3];
    return o;
}

mat4 multiplyMat4(const mat4 &m1, const mat4 &m2)
{
    mat4 out;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            out.m[r][c] = m1.m[r][0] * m2.m[0][c]
                        + m1.m[r][1] * m2.m[1][c]
                        + m1.m[r][2] * m2.m[2][c]
                        + m1.m[r][3] * m2.m[3][c];
    return out;
}

// http://www.graphics.stanford.edu/courses/cs248-98-fall/Final/q4.html
mat4 inverseMatrixSimple(const mat4 &m) // Only for Rotation/Translation Matrices
{
    mat4 out;
    // row order: matrix[row][col]
    // The basic idea is that the scaling/rotation combination of the transformation matrix
    // (first 3x3 sub-matrix) is an orthonormal matrix
    // and inverse of on orthonormal matrix is equal to the the transpose.
    out.m[0][0] = m.m[0][0];  out.m[0][1] = m.m[1][0];  out.m[0][2] = m.m[2][0];
    out.m[1][0] = m.m[0][1];  out.m[1][1] = m.m[1][1];  out.m[1][2] = m.m[2][1];
    out.m[2][0] = m.m[0][2];  out.m[2][1] = m.m[1][2];  out.m[2][2] = m.m[2][2];
    out.m[3][0] = 0.0f;       out.m[3][1] = 0.0f;       out.m[3][2] = 0.0f;

    float tx = m.m[0][3];
    float ty = m.m[1][3];
    float tz = m.m[2][3];
    out.m[0][3] = -(m.m[0][0] * tx + m.m[0][1] * ty + m.m[0][2] * tz);
    out.m[1][3] = -(m.m[1][0] * tx + m.m[1][1] * ty + m.m[1][2] * tz);
    out.m[2][3] = -(m.m[2][0] * tx + m.m[2][1] * ty + m.m[2][2] * tz);
    out.m[3][3] = 1.0f;
    return out;
}

// TODO: check if these are still valid references/notes
// https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/lookat-function (looks flaky)
// https://github.com/g-truc/glm/blob/master/glm/ext/matrix_transform.inl#L99 (lookAtRH)
// and finally https://www.3dgep.com/understanding-the-view-matrix/
//
// After the camera lookat transformation, the camera is at origin
// looking at negative Z-axis (so the z-axis is flipped!)
//
// NOTE: https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/lookat-function
// When the camera is vertical looking straight down or straight up, the forward axis gets very close
// to the arbitrary axis used to compute the right axis. The extreme case is of course when the froward
// axis and this arbitrary axis are perfectly parallel e.g. when the forward vector is either (0,1,0)
// or (0,-1,0). Unfortunately in this particular case, the cross product fails producing a result for
// the right vector.

// https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/lookat-function
mat4 lookAtMatrixRH(const vec3 &eye, const vec3 &center, const vec3 &tmp = vec3{0.0f, 1.0f, 0.0f})
{
    // NOTE RIGHT-HANDED!!!
    vec3 forward = v3Normalize(v3Sub(eye, center)); // camera looks towards negative z-axis still
    // printVec3(forward);
    vec3 right = v3Normalize(v3CrossProduct(tmp, forward));
    // printVec3(right);
    vec3 up = v3Normalize(v3CrossProduct(forward, right));
    // printVec3(up);

    // See: https://stackoverflow.com/questions/349050/calculating-a-lookat-matrix
    // and https://docs.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatrh
    mat4 camToWorld {
        {
            {   right.x,     right.y,    right.z, -v3DotProduct(right, eye)},
            {      up.x,        up.y,       up.z, -v3DotProduct(up, eye)},
            { forward.x,   forward.y,  forward.z, -v3DotProduct(forward, eye)},
            {      0.0f,        0.0f,       0.0f,                         1.0f}
        }
    };
    // printMat4(camToWorld);

    return camToWorld;
}

// Pitch must be in the range of [-90 ... 90] degrees and
// yaw must be in the range of [0 ... 360] degrees.
// Pitch and yaw variables must be expressed in radians.
mat4 fpsLookAtMatrixRH(vec3 eye, float pitch, float yaw)
{
    // I assume the values are already converted to radians.
    float cosPitch = cosf(pitch);
    float sinPitch = sinf(pitch);
    float cosYaw = cosf(yaw);
    float sinYaw = sinf(yaw);

    vec3 xaxis = {cosYaw, 0, -sinYaw};
    printVec3(xaxis);
    vec3 yaxis = {sinYaw * sinPitch, cosPitch, cosYaw * sinPitch};
    printVec3(yaxis);
    vec3 zaxis = {sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw};
    printVec3(zaxis);

    mat4 fpsViewMatrix {
        {
            { xaxis.x,   xaxis.y,  xaxis.z, -v3DotProduct( xaxis, eye )},
            { yaxis.x,   yaxis.y,  yaxis.z, -v3DotProduct( yaxis, eye )},
            { zaxis.x,   zaxis.y,  zaxis.z, -v3DotProduct( zaxis, eye )},
            {    0.0f,      0.0f,     0.0f,                        1.0f}
        }
    };

    return fpsViewMatrix;
}

float MINOR(float m[16], int r0, int r1, int r2, int c0, int c1, int c2)
{
    return m[4 * r0 + c0] * (m[4 * r1 + c1] * m[4 * r2 + c2] - m[4 * r2 + c1] * m[4 * r1 + c2]) -
           m[4 * r0 + c1] * (m[4 * r1 + c0] * m[4 * r2 + c2] - m[4 * r2 + c0] * m[4 * r1 + c2]) +
           m[4 * r0 + c2] * (m[4 * r1 + c0] * m[4 * r2 + c1] - m[4 * r2 + c0] * m[4 * r1 + c1]);
}

void adjoint(float m[16], float adjOut[16])
{
    adjOut[ 0] =  MINOR(m,1,2,3,1,2,3); adjOut[ 1] = -MINOR(m,0,2,3,1,2,3); adjOut[ 2] =  MINOR(m,0,1,3,1,2,3); adjOut[ 3] = -MINOR(m,0,1,2,1,2,3);
    adjOut[ 4] = -MINOR(m,1,2,3,0,2,3); adjOut[ 5] =  MINOR(m,0,2,3,0,2,3); adjOut[ 6] = -MINOR(m,0,1,3,0,2,3); adjOut[ 7] =  MINOR(m,0,1,2,0,2,3);
    adjOut[ 8] =  MINOR(m,1,2,3,0,1,3); adjOut[ 9] = -MINOR(m,0,2,3,0,1,3); adjOut[10] =  MINOR(m,0,1,3,0,1,3); adjOut[11] = -MINOR(m,0,1,2,0,1,3);
    adjOut[12] = -MINOR(m,1,2,3,0,1,2); adjOut[13] =  MINOR(m,0,2,3,0,1,2); adjOut[14] = -MINOR(m,0,1,3,0,1,2); adjOut[15] =  MINOR(m,0,1,2,0,1,2);
}

float det(float m[16])
{
    float out = m[0] * MINOR(m, 1, 2, 3, 1, 2, 3) -
                m[1] * MINOR(m, 1, 2, 3, 0, 2, 3) +
                m[2] * MINOR(m, 1, 2, 3, 0, 1, 3) -
                m[3] * MINOR(m, 1, 2, 3, 0, 1, 2);
    if (out == 0)
        std::cerr << "DET 0!!!" << std::endl;
    return out;
}

// http://rodolphe-vaillant.fr/?e=7
void invertRowMajor(float m[16], float invOut[16])
{
    adjoint(m, invOut);

    float inv_det = 1.0f / det(m);
    for (int i = 0; i < 16; ++i)
        invOut[i] = invOut[i] * inv_det;
}

mat4 invertMat4(const mat4 &matrix)
{
    mat4 inverse;
    invertRowMajor((float *)matrix.m, (float *)inverse.m);
    return inverse;
}

mat4 transposeMat4(const mat4 &matrix)
{
    mat4 transpose{};
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
            transpose.m[j][i] = matrix.m[i][j];
    }
    return transpose;
}

void printMat4(const mat4 &mat, std::ostream& os)
{
    for (int r : {0, 1, 2, 3})
    {
        for (int c : {0, 1, 2, 3})
        {
            os << " " << mat.m[r][c];
        }
        os << std::endl;
    }
}
void printVec3(const vec3 &v)
{
    std::cout << "( " << v.x << " " << v.y << " " << v.z << " " << v.w << " )" << std::endl;
}

void printTri(const tri &t, std::string label = "")
{
    std::cout << "TRI (" << label << "):";
    for (int i = 0; i < 3; i++)
        std::cout << " (" << t.vertices[i].x << " " << t.vertices[i].y << " " << t.vertices[i].z << " " << t.vertices[i].w << ")";
    std::cout << std::endl;
}

u_int32_t adjustColor(u_int32_t color, float multiplier)
{
    u_int8_t r = (u_int8_t)(color >> 24);
    u_int8_t g = (u_int8_t)(color >> 16);
    u_int8_t b = (u_int8_t)(color >> 8);
    u_int8_t a = (u_int8_t)(color >> 0);

    r *= multiplier;
    g *= multiplier;
    b *= multiplier;

    return (r << 24) | (g << 16) | (b << 8) | a;
}
void printColor(u_int32_t color)
{
    u_int8_t r = (u_int8_t)(color >> 24);
    u_int8_t g = (u_int8_t)(color >> 16);
    u_int8_t b = (u_int8_t)(color >> 8);
    u_int8_t a = (u_int8_t)(color >> 0);
    std::cout << "color r:" << std::to_string(r)
        << " color b:" << std::to_string(b)
        << " color g:" << std::to_string(g)
        << " color a:" << std::to_string(a) << std::endl;
}
#endif // MATH_TYPES_ONLY

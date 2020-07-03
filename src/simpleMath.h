#pragma once

#include <cmath>
#include <string>
#include <vector>

struct vec3
{
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;
    // structs don't have default comparison
    bool operator==(const vec3& other) const
    {
        return this->x == other.x
            && this->y == other.y
            && this->z == other.z
            && this->w == other.w;
    };
};

struct tri
{
	vec3 vertices[3];
};

struct mesh
{
	std::vector<tri> tris;
    vec3 scale{1.0f, 1.0f, 1.0f};
    vec3 rotation{};
    vec3 translation{};
};

struct mat4
{
	float m[4][4] = {{0}};
    bool operator==(const mat4& other) const
    {
        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
            {
                if (this->m[r][c] != other.m[r][c])
                    return false;
            }
        }
        return true;
    };
};

// forward declarations:
void printMat4(const mat4 &mat);
void printVec3(const vec3 &v);
void printTri(const tri &t, std::string label);


vec3 v3Add(const vec3 &v1, const vec3 &v2)
{
    return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

vec3 v3Sub(const vec3 &v1, const vec3 &v2)
{
    return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

vec3 v3Mul(const vec3 &v1, const float k)
{
    return { v1.x * k, v1.y * k, v1.z * k };
}

vec3 v3Div(const vec3 &v1, const float k)
{
    return { v1.x / k, v1.y / k, v1.z / k };
}

float v3DotProduct(const vec3 &v1, const vec3 &v2)
{
    return v1.x*v2.x + v1.y*v2.y + v1.z * v2.z;
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
    return { v.x / l, v.y / l, v.z / l };
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
mat4 projectionMatrix(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
{
    float fFovRad = tanf(fFovDegrees * 0.5f * 3.14159f / 180.0f );

    mat4 out;
    // row order: matrix[row][col]
    out.m[0][0] = 1 / (fAspectRatio * fFovRad);
    out.m[1][1] = 1 / fFovRad;
    out.m[2][2] = (-fNear - fFar) / (fNear - fFar);
    out.m[2][3] = (2 * fFar * fNear) / (fNear - fFar);
    out.m[3][2] = 1.0f;
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
    mat4  out;
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
    out.m[0][3] = -(m.m[0][0]*tx + m.m[0][1]*ty + m.m[0][2]*tz);
    out.m[1][3] = -(m.m[1][0]*tx + m.m[1][1]*ty + m.m[1][2]*tz);
    out.m[2][3] = -(m.m[2][0]*tx + m.m[2][1]*ty + m.m[2][2]*tz);
    out.m[3][3] = 1.0f;
    return  out;
}

// https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/lookat-function (looks flaky)
// https://github.com/g-truc/glm/blob/master/glm/ext/matrix_transform.inl#L99 (lookAtRH)
// and finally https://www.3dgep.com/understanding-the-view-matrix/
mat4 lookAtMatrixRH(const vec3 &from, const vec3 &to, const vec3 &tmp = vec3{0.0f, 1.0f, 0.0f})
{   // NOTE RIGHT-HANDED!!!
    vec3 forward = v3Normalize(v3Sub(to, from));// !!! switched !!! normalize(from - to);
    // printVec3(forward);
    vec3 right = v3Normalize(v3CrossProduct(forward, tmp)); // !!! switched !!! crossProduct(normalize(tmp), forward);
    // printVec3(right);
    vec3 up = v3CrossProduct(right, forward); // !!! switched !!! crossProduct(forward, right);
    // printVec3(up);

    mat4 camToWorld;

    camToWorld.m[0][0] = right.x;
    camToWorld.m[0][1] = right.y;
    camToWorld.m[0][2] = right.z;
    camToWorld.m[1][0] = up.x;
    camToWorld.m[1][1] = up.y;
    camToWorld.m[1][2] = up.z;
    camToWorld.m[2][0] = -forward.x; // !!! negate !!!
    camToWorld.m[2][1] = -forward.y; // !!! negate !!!
    camToWorld.m[2][2] = -forward.z; // !!! negate !!!

    camToWorld.m[0][3] = -v3DotProduct(right, from); // !!! from.x;
    camToWorld.m[1][3] = -v3DotProduct(up, from); // !!! from.y;
    camToWorld.m[2][3] = -v3DotProduct(forward, from); // !!! from.z;

    camToWorld.m[3][3] = 1.0f; // GUESS!!!

    return camToWorld;
}

// Pitch must be in the range of [-90 ... 90] degrees and
// yaw must be in the range of [0 ... 360] degrees.
// Pitch and yaw variables must be expressed in radians.
mat4 fpsLookAtMatrixRH( vec3 eye, float pitch, float yaw )
{
    // I assume the values are already converted to radians.
    float cosPitch = cos(pitch);
    float sinPitch = sin(pitch);
    float cosYaw = cos(yaw);
    float sinYaw = sin(yaw);

    vec3 xaxis = { cosYaw, 0, -sinYaw };
    vec3 yaxis = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
    vec3 zaxis = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };

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
    return m[4*r0+c0] * (m[4*r1+c1] * m[4*r2+c2] - m[4*r2+c1] * m[4*r1+c2]) -
           m[4*r0+c1] * (m[4*r1+c0] * m[4*r2+c2] - m[4*r2+c0] * m[4*r1+c2]) +
           m[4*r0+c2] * (m[4*r1+c0] * m[4*r2+c1] - m[4*r2+c0] * m[4*r1+c1]);
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
    for(int i = 0; i < 16; ++i)
        invOut[i] = invOut[i] * inv_det;
}


void printMat4(const mat4 &mat)
{
    for (int r : {0, 1, 2, 3})
    {
        for (int c : {0, 1, 2, 3})
        {
            std::cout << " " << mat.m[r][c];
        }
        std::cout << std::endl;
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
        std::cout <<  " (" << t.vertices[i].x << " " << t.vertices[i].y << " " << t.vertices[i].z << " " << t.vertices[i].w << ")";
    std::cout << std::endl;
}

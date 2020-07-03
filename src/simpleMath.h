#pragma once

#include <cmath>
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

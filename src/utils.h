
#include <cmath>
#include <vector>

// todo: sort out include order (vec3, tri and mesh from simpleMath.h)

// adapted from https://github.com/caosdoar/spheres/blob/master/src/spheres.cpp
void sphere(size_t meridians, size_t parallels, u_int32_t color, mesh &mesh)
{
	std::vector<vec3> vertices;

    // top / north pole
    vertices.emplace_back(vec3{0.0f, 1.0f, 0.0f});

    for (size_t j = 0; j < parallels - 1; ++j)
	{
		float const polar = M_PI * float(j+1) / float(parallels);
		float const sp = std::sin(polar);
		float const cp = std::cos(polar);
		for (size_t i = 0; i < meridians; ++i)
		{
			float const azimuth = 2.0 * M_PI * float(i) / float(meridians);
			float const sa = std::sin(azimuth);
			float const ca = std::cos(azimuth);
			float const x = sp * ca;
			float const y = cp;
			float const z = sp * sa;
			vertices.emplace_back(vec3{x, y, z});
		}
	}

    // bottom / south pole
    vertices.emplace_back(vec3{0.0f, -1.0f, 0.0f});

	// top triangles
    for (size_t i = 0; i < meridians; ++i)
	{
		size_t const a = i + 1;
		size_t const b = (i + 1) % meridians + 1;
		mesh.tris.push_back(tri{
            {vertices[0], vertices[a], vertices[b]},
            color
        });
	}

	for (size_t j = 0; j < parallels - 2; ++j)
	{
		size_t aStart = j * meridians + 1;
		size_t bStart = (j + 1) * meridians + 1;
		for (size_t i = 0; i < meridians; ++i)
		{
			const size_t a = aStart + i;
			const size_t a1 = aStart + (i + 1) % meridians;
			const size_t b = bStart + i;
			const size_t b1 = bStart + (i + 1) % meridians;
			// mesh.addQuad(a, a1, b1, b);
            mesh.tris.push_back(tri{
                {vertices[b], vertices[a1], vertices[a]}, color
            });
            mesh.tris.push_back(tri{
                {vertices[b], vertices[b1], vertices[a1]}, color
            });
		}
	}

    // bottom triangles
	for (size_t i = 0; i < meridians; ++i)
	{
		size_t const a = i + meridians * (parallels - 2) + 1;
		size_t const b = (i + 1) % meridians + meridians * (parallels - 2) + 1;
		//mesh.addTriangle(mesh.vertices.size() - 1, a, b);
        mesh.tris.push_back(tri{
            {vertices[vertices.size() - 1], vertices[b], vertices[a]}, color
        });
	}
}

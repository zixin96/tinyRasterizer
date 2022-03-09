#include "tinyOpenGL.h"

#include <glm/ext/scalar_constants.hpp>


glm::mat4 ModelView;
glm::mat4 Projection;

IShader::~IShader()
{
}

void lookat(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& tmp)
{
	glm::vec3 forward = glm::normalize((eye - center));
	glm::vec3 right = glm::cross(glm::normalize(tmp), forward);
	glm::vec3 up = glm::cross(forward, right);

	// In glm we access elements as mat[col][row] due to column-major layout

	// OpenGL camToWorld matrix: (different from SAP because matrices in OpenGL are defined using a column-major order) 
	// [ Right.x Up.x Forward.x T.x ]
	// [ Right.y Up.y Forward.y T.y ]
	// [ Right.z Up.z Forward.z T.z ]
	// [ 0       0    0         1.0 ]

	// When accessing as column-major layout, the first curly braces should contain the first column of this matrix

	glm::mat4 camToWorld = {
		{right.x, right.y, right.z, 0.f},
		{up.x, up.y, up.z, 0.f},
		{forward.x, forward.y, forward.z, 0.f},
		{eye.x, eye.y, eye.z, 1.f},
	};

	ModelView = glm::inverse(camToWorld);
}


void projection(const float& fovy, const float& aspect, const float& near, const float& far)
{
	// compute r,l,b,t (screen coordinates boundary)
	float r, l, b, t;
	float scale = glm::tan(fovy * 0.5f * glm::pi<float>() / 180.f) * near;
	r = aspect * scale, l = -r;
	t = scale, b = -t;

	// OpenGL Perspective Projection Matrix
	// [ 2n/(r-l) 0			(r+l)/(r-l)		0			]
	// [ 0        2n/(t-b)	(t+b)/(t-b)		0			]
	// [ 0		  0			-(f+n)/(f-n)	-2fn/(f-n)	]
	// [ 0		  0			-1				0			]
	glm::mat4 persp = {
		{2 * near / (r - l), 0.f, 0.f, 0.f},
		{0.f, 2 * near / (t - b), 0.f, 0.f},
		{(r + l) / (r - l), (t + b) / (t - b), -(far + near) / (far - near), -1.f},
		{0.f, 0.f, -2 * far * near / (far - near), 0.f},
	};

	Projection = persp;
}

static float min3(const float& a, const float& b, const float& c)
{
	return std::min(a, std::min(b, c));
}

static float max3(const float& a, const float& b, const float& c)
{
	return std::max(a, std::max(b, c));
}

static float edgeFunction(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
	return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}

void triangle(glm::vec4* hcp, IShader& shader, TGAImage& image, std::vector<float>& zbuffer)
{
	uint32_t imageWidth = image.get_width();
	uint32_t imageHeight = image.get_height();
	// clipping (ignore)
	// perspective divide 
	glm::vec3 ndc[3] = {
		{hcp[0].x / hcp[0].w, hcp[0].y / hcp[0].w, hcp[0].z / hcp[0].w},
		{hcp[1].x / hcp[1].w, hcp[1].y / hcp[1].w, hcp[1].z / hcp[1].w},
		{hcp[2].x / hcp[2].w, hcp[2].y / hcp[2].w, hcp[2].z / hcp[2].w},
	};
	// viewport transform
	// note: we set the raster depth to be NDC depth
	glm::vec3 raster[3] = {
		{(ndc[0].x + 1) / 2 * imageWidth, (1 - ndc[0].y) / 2 * imageHeight, ndc[0].z},
		{(ndc[1].x + 1) / 2 * imageWidth, (1 - ndc[1].y) / 2 * imageHeight, ndc[1].z},
		{(ndc[2].x + 1) / 2 * imageWidth, (1 - ndc[2].y) / 2 * imageHeight, ndc[2].z},
	};

	// precompute reciprocal of vertex z-coordinate
	raster[0].z = 1 / raster[0].z;
	raster[1].z = 1 / raster[1].z;
	raster[2].z = 1 / raster[2].z;

	// prepare vertex attributes: divide them by their vertex z-coordinate   

	// compute bounding box of this triangle
	float xmin = min3(raster[0].x, raster[1].x, raster[2].x);
	float ymin = min3(raster[0].y, raster[1].y, raster[2].y);
	float xmax = max3(raster[0].x, raster[1].x, raster[2].x);
	float ymax = max3(raster[0].y, raster[1].y, raster[2].y);

	// the triangle is out of screen
	if (xmin > imageWidth - 1 || xmax < 0 || ymin > imageHeight - 1 || ymax < 0)
	{
		return;
	}

	// be careful xmin/xmax/ymin/ymax can be negative. Don't cast to uint32_t
	// some bounding box coordinates may be outside the range, clamp them if necessary
	uint32_t x0 = std::max(0, static_cast<int32_t>(std::floor(xmin)));
	uint32_t x1 = std::min(static_cast<int32_t>(imageWidth) - 1, static_cast<int32_t>(std::floor(xmax)));
	uint32_t y0 = std::max(0, static_cast<int32_t>(std::floor(ymin)));
	uint32_t y1 = std::min(static_cast<int32_t>(imageHeight) - 1, static_cast<int32_t>(std::floor(ymax)));

	float area = edgeFunction(raster[0], raster[1], raster[2]);

	for (uint32_t y = y0; y <= y1; ++y)
	{
		for (uint32_t x = x0; x <= x1; ++x)
		{
			glm::vec3 pixelSample(x + 0.5, y + 0.5, 0);

			float w0 = edgeFunction(raster[1], raster[2], pixelSample);
			float w1 = edgeFunction(raster[2], raster[0], pixelSample);
			float w2 = edgeFunction(raster[0], raster[1], pixelSample);
			// test if this pixel sample covers this triangle
			if (w0 >= 0 && w1 >= 0 && w2 >= 0)
			{
				// compute the pixel barycentric coordinates
				w0 /= area;
				w1 /= area;
				w2 /= area;
				// compute the (1/depth) of this pixel by linearly interpolating\
				// the reciprocal of 3 vertices' depth (precomputed outside the loop)
				// and then take the reciprocal to get the resulting depth
				float oneOverZ = raster[0].z * w0 + raster[1].z * w1 + raster[2].z * w2;
				float z = 1 / oneOverZ;
				// Depth-buffer test
				if (z < zbuffer[y * imageWidth + x])
				{
					TGAColor color;
					glm::vec4 baryCoordAndPixeldepth = glm::vec4(w0, w1, w2, z);
					if (shader.fragment(baryCoordAndPixeldepth, color))
					{
						// fragment shader can discard this pixel
						continue;
					}
					zbuffer[y * imageWidth + x] = z;
					image.set(x, y, color);
				}
			}
		}
	}
}

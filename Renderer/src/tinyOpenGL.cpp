#include "tinyOpenGL.h"

glm::mat4 ModelView;
glm::mat4 Viewport;
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

	// Our camToWorld matrix is: (different from SAP because SAP uses row-major matrices)
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


void viewport(int x, int y, int w, int h)
{
}

void projection(float coeff)
{
}

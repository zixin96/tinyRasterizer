#pragma once
#include <glm/glm.hpp>

// from world to camera space (equivalent to glm::lookAt)
void lookat(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& tmp = glm::vec3(0.f, 1.f, 0.f));
// from camera to canvas space
void projection(float coeff = 0.f); // coeff = -1/c

void viewport(int x, int y, int w, int h);

struct IShader
{
	virtual ~IShader();
	virtual void vertex(const int iface, const int nthvert, glm::vec4& gl_Position) = 0;
	virtual bool fragment(const glm::vec3& bar, glm::vec4& color) = 0;
};

// void triangle(glm::vec4 * pts, IShader & shader, TGAImage & image, TGAImage & zbuffer);

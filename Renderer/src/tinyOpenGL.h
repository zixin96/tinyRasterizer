#pragma once
#include <glm/glm.hpp>

extern glm::mat4 ModelView;
extern glm::mat4 Viewport;
extern glm::mat4 Projection;

void viewport(int x, int y, int w, int h);
void projection(float coeff = 0.f); // coeff = -1/c
void lookat(glm::vec3 eye, glm::vec3 center, glm::vec3 up);

struct IShader
{
	virtual ~IShader();
	virtual glm::vec4 vertex(int iface, int nthvert) = 0;
	virtual bool fragment(const glm::vec3& bar, glm::vec4& color) = 0;
};

// void triangle(glm::vec4 * pts, IShader & shader, TGAImage & image, TGAImage & zbuffer);

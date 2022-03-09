#pragma once
#include <glm/glm.hpp>

#include "Mesh.h"
#include <tgaimage.h>

// from world to camera space (equivalent to glm::lookAt)
void lookat(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& tmp = glm::vec3(0.f, 1.f, 0.f));
// from camera to homogeneous clip space 
void projection(const float& fovy, const float& aspect, const float& near, const float& far);

struct IShader
{
	virtual ~IShader();
	virtual void vertex(const Vertex& v, const int nthVert, glm::vec4& gl_Position) = 0;
	virtual bool fragment(const glm::vec4& bar, TGAColor& gl_FragColor) = 0;
};

// void triangle(glm::vec4 * pts, IShader & shader, TGAImage & image, TGAImage & zbuffer);
void triangle(glm::vec4* hcp, IShader& shader, TGAImage& image, std::vector<float>& zbuffer);

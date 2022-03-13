#pragma once
#include <glm/glm.hpp>

#include "Mesh.h"
#include <tgaimage.h>
#include <unordered_map>


// from world to camera space (equivalent to glm::lookAt)
void lookat(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& tmp = glm::vec3(0.f, 1.f, 0.f));
// from camera to homogeneous clip space (equivalent to glm::perspective) 
void projection(const float& fovy, const float& aspect, const float& near, const float& far);

// IShader encapsulates tinyOpenGL System 
struct IShader
{
	virtual ~IShader();
	virtual bool fragment(const glm::vec4& bar, TGAColor& gl_FragColor, float r0z, float r1z, float r2z) = 0;

	static TGAColor sample2D(const TGAImage& img, glm::vec2& uvf)
	{
		return img.get(uvf[0] * img.width(), uvf[1] * img.height());
	}

	// textUnit: specifies the texture unit number
	// uvf: specifies the texture coordinates
	// return: the texel color for the coordinates
	static TGAColor texture2D(unsigned textUnit, glm::vec2& uvf)
	{
		const TGAImage& img = tinyOpenGLTextures[textUnit];
		return img.get(uvf[0] * img.width(), uvf[1] * img.height());
	}

	static std::unordered_map<unsigned, TGAImage> tinyOpenGLTextures;
};

// this function 
// 1) covers the geometric shape assembly process (Primitive Assembly): note we only support gl.TRIANGLES 
// 2) covers the rasterization process (Rasterizer): the geometric shape assembled in the geometric assembly process is converted into fragments  
// 3) calls fragment shader
void triangle(glm::vec4* hcp, IShader& shader, TGAImage& image, std::vector<float>& zbuffer);

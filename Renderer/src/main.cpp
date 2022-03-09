#include "Model.h"
#include "tinyOpenGL.h"
#include <glm/gtx/string_cast.hpp>

#include "tgaimage.h"


const glm::vec3 light_dir(1, 1, 1);
const glm::vec3 eye(1, 1, 3);
const glm::vec3 center(0, 0, 0);
const glm::vec3 up(0, 1, 0);

extern glm::mat4 ModelView; // "OpenGL" state matrices
extern glm::mat4 Projection;

struct Shader : IShader
{
	const Model& model;

	// all varying attributes are written by the vertex shader, read by the fragment shader

	// triangle vertex: v1, v2, v3 has the following uv structure
	// [ v1.u v2.u v3.u ]
	// [ v1.v v2.v v3.v ]
	// glm::mat3x2 varying_uv;

	glm::vec3 varying_intensity;

	Shader(const Model& m) : model(m)
	{
	}

	virtual void vertex(const Vertex& v, const int nthVert, glm::vec4& gl_Position)
	{
		// varying_uv[nthVert] = v.TexCoords;
		varying_intensity[nthVert] = std::max(0.f, glm::dot(v.Normal, glm::normalize(light_dir)));
		// shall we normalize normal and light_dir? 
		gl_Position = Projection * ModelView * glm::vec4(v.Position, 1.f);
	}

	virtual bool fragment(const glm::vec4& bar, TGAColor& gl_FragColor)
	{
		// compute interpolated attributes
		float pixelDepth = bar.w;
		float intensity = glm::dot(varying_intensity, glm::vec3(bar)) * pixelDepth;
		gl_FragColor = TGAColor(255, 255, 255) * intensity;
		return false; // the pixel is not discarded
	}
};

// our main() function is the primitive processing routine. It calls the vertex shader. 
// We do not have primitive assembly here, since we are drawing dumb triangles only (in our code it is merged with the primitive processing). 
int main()
{
	// load models
	// use "/" for file path
	// Model ourModel("assets/viking_room_gltf/scene.gltf");
	Model ourModel("assets/obj/african_head/african_head.obj");
	Shader shader(ourModel);

	uint32_t imageWidth = 800;
	uint32_t imageHeight = 800;
	float aspect = imageWidth / (float)imageHeight;
	float fovy = 60.f;
	float near = 0.1f;
	float far = 100.f;
	// initialize lookat and projecton matrix
	lookat(eye, center);
	projection(fovy, aspect, near, far);

	TGAImage framebuffer(imageWidth, imageHeight, TGAImage::RGB);
	std::vector<float> zbuffer(imageWidth * imageHeight, std::numeric_limits<float>::max());

	// iterate through all meshes
	for (const auto& mesh : ourModel.meshes)
	{
		// iterate through each triangle in the mesh
		for (size_t i = 0; i < mesh.indices.size(); i += 3)
		{
			glm::vec4 homogeneousClipSpace[3];
			for (int j = 0; j < 3; j++)
			{
				shader.vertex(mesh.vertices[mesh.indices[i + j]], j, homogeneousClipSpace[j]);
			}
			triangle(homogeneousClipSpace, shader, framebuffer, zbuffer);
		}
	}

	framebuffer.write_tga_file("output2.tga");
	return 0;
}

#include "Model.h"
#include "tinyOpenGL.h"
#include <glm/gtx/string_cast.hpp>

#include "tgaimage.h"


const glm::vec3 eye(1, 1, 3);
const glm::vec3 center(0, 0, 0);
const glm::vec3 up(0, 1, 0);

extern glm::mat4 View; // "OpenGL" state matrices
extern glm::mat4 Projection;

#if 0
struct Shader : IShader
{
	const Model& model;

	// all varying attributes are written by the vertex shader, read by the fragment shader

	// triangle vertex: v1, v2, v3 has the following uv structure
	// [ v1.u v2.u v3.u ]
	// [ v1.v v2.v v3.v ]
	glm::mat3x2 varying_uv;
	glm::vec3 varying_intensity;

	Shader(const Model& m) : model(m)
	{
	}

	virtual void vertex(const Vertex& v, const int nthVert, glm::vec4& gl_Position)
	{
		varying_uv[nthVert] = v.TexCoords;
		varying_intensity[nthVert] = std::max(0.f, glm::dot(v.Normal, glm::normalize(light_dir)));
		gl_Position = Projection * ModelView * glm::vec4(v.Position, 1.f);
	}

	virtual bool fragment(const glm::vec4& bar, TGAColor& gl_FragColor, float r0z, float r1z, float r2z)
	{
		// compute interpolated attributes (In real OpenGL, these attributes are interpolated before fragment shader executes)
		float intensity = bar.w * (varying_intensity[0] * r0z * bar[0]
			+ varying_intensity[1] * r1z * bar[1]
			+ varying_intensity[2] * r2z * bar[2]);

		glm::vec2 uv = bar.w * (varying_uv[0] * r0z * bar[0]
			+ varying_uv[1] * r1z * bar[1]
			+ varying_uv[2] * r2z * bar[2]);

		TGAColor c = sample2D(model.textures_loaded[0].data, uv);
		gl_FragColor = c * intensity;
		return false; // the pixel is not discarded
	}
};
#endif

struct Shader : IShader
{
	const Mesh& mesh;
	glm::mat4 uniform_Model;
	glm::mat4 uniform_NormalMat;
	glm::mat4 uniform_View;
	glm::mat4 uniform_Projection;
	glm::vec3 uniform_LightDir;

	// all varying attributes are written by the vertex shader, read by the fragment shader

	// triangle vertex: v1, v2, v3 has the following uv structure
	// [ v1.u v2.u v3.u ]
	// [ v1.v v2.v v3.v ]
	glm::mat3x2 varying_uv;

	Shader(const Mesh& m) : mesh(m)
	{
	}

	// a_XXX represents vertex attribute
	// nthVertex is needed for varying attributes
	void vertex(const glm::vec3& a_Position, const glm::vec2& a_TexCoord, const int nthVert,
	            glm::vec4& gl_Position)
	{
		varying_uv[nthVert] = a_TexCoord;
		gl_Position = uniform_Projection * uniform_View * uniform_Model * glm::vec4(a_Position, 1.f);
	}

	virtual bool fragment(const glm::vec4& bar, TGAColor& gl_FragColor, float r0z, float r1z, float r2z)
	{
		// compute interpolated attributes (In real OpenGL, these attributes are interpolated before fragment shader executes)
		glm::vec2 uv = bar.w * (varying_uv[0] * r0z * bar[0]
			+ varying_uv[1] * r1z * bar[1]
			+ varying_uv[2] * r2z * bar[2]);

		TGAColor diffuseValue{};
		TGAColor specularValue{};
		TGAColor normalValue{};
		glm::vec3 n{};
		for (const auto& tex : mesh.textures)
		{
			if (tex.type == "texture_diffuse")
			{
				diffuseValue = sample2D(tex.data, uv);
			}
			else if (tex.type == "texture_normal")
			{
				normalValue = sample2D(tex.data, uv);
				// convert normal from [0, 255] to [-1,1]
				n = glm::vec3(normalValue[0], normalValue[1], normalValue[2]) * 2.f / 255.f -
					glm::vec3(1.f, 1.f, 1.f);
			}
			else if (tex.type == "texture_specular")
			{
				specularValue = sample2D(tex.data, uv);
			}
		}

		// diffuse
		glm::vec3 norm = glm::normalize(uniform_NormalMat * glm::vec4(n, 0.f));
		glm::vec3 lightDir = glm::normalize(uniform_Model * glm::vec4(uniform_LightDir, 0.f));
		float diff = std::max(glm::dot(norm, lightDir), 0.f);

		// specular
		glm::vec3 r = glm::reflect(-lightDir, norm);
		float spec = std::pow(std::max(r.z, 0.f), specularValue[0]);
		TGAColor c = diffuseValue;
		for (int i : {0, 1, 2})
			gl_FragColor[i] = std::min<int>(5 + c[i] * (diff + 1.5f * spec), 255);

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

	uint32_t imageWidth = 800;
	uint32_t imageHeight = 800;
	float aspect = imageWidth / (float)imageHeight;
	float fovy = 50.f;
	float near = 0.1f;
	float far = 100.f;
	// initialize lookat and projecton matrix
	lookat(eye, center);
	projection(fovy, aspect, near, far);

	TGAImage framebuffer(imageWidth, imageHeight, TGAImage::RGB);
	std::vector<float> zbuffer(imageWidth * imageHeight, std::numeric_limits<float>::max());

	// iterate through all meshes
	for (size_t m = 0; m < ourModel.meshes.size(); m++)
	{
		Shader shader(ourModel.meshes[m]);
		// we want our mesh to be where it is originally 
		glm::mat4 Model = glm::mat4(1.f);
		shader.uniform_Model = Model;
		shader.uniform_NormalMat = glm::transpose(glm::inverse(Model));
		shader.uniform_View = View;
		shader.uniform_Projection = Projection;
		shader.uniform_LightDir = glm::vec3(1.f, 1.f, 0.5f);
		// iterate through each triangle in the mesh
		for (size_t i = 0; i < ourModel.meshes[m].indices.size(); i += 3)
		{
			glm::vec4 homogeneousClipSpace[3];
			for (int j = 0; j < 3; j++)
			{
				const Vertex& v = ourModel.meshes[m].vertices[ourModel.meshes[m].indices[i + j]];
				shader.vertex(v.Position, v.TexCoords, j,
				              homogeneousClipSpace[j]);
			}
			triangle(homogeneousClipSpace, shader, framebuffer, zbuffer);
		}
	}

	framebuffer.write_tga_file("2.tga", false);
	return 0;
}

#include "Model.h"
#include "tinyOpenGL.h"
#include <glm/gtx/string_cast.hpp>
constexpr int width = 800; // output image size
constexpr int height = 800;

const glm::vec3 light_dir(1, 1, 1);
const glm::vec3 eye(0, -1, 3);
const glm::vec3 center(0, 0, 0);
const glm::vec3 up(0, 1, 0);

extern glm::mat4 ModelView; // "OpenGL" state matrices
extern glm::mat4 Projection;

struct Shader : IShader
{
	const Model& model;

	Shader(const Model& m) : model(m)
	{
	}

	virtual void vertex(const int iface, const int nthvert, glm::vec4& gl_Position)
	{
		gl_Position = glm::vec4();
	}

	virtual bool fragment(const glm::vec3& bar, glm::vec4& gl_FragColor)
	{
		return false; // the pixel is not discarded
	}
};

// our main() function is the primitive processing routine. It calls the vertex shader. 
// We do not have primitive assembly here, since we are drawing dumb triangles only (in our code it is merged with the primitive processing). 
int main()
{
	// load models
	// use "/" for file path
	Model ourModel("assets/viking_room_gltf/scene.gltf");
	Shader shader(ourModel);

	// Initialization of ModelView, Projection and Viewport matrices (recall that actual instances of these matrices are in the our_gl module)
	lookat(eye, center);

	glm::mat4 correctLookAt = glm::lookAt(eye, center, glm::vec3(0.f, 1.f, 0.f));
	std::cout << glm::to_string(ModelView) << std::endl;
	std::cout << '\n';
	std::cout << glm::to_string(correctLookAt) << std::endl;

	return 0;
}

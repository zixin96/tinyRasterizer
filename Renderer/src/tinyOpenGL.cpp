#include "tinyOpenGL.h"

glm::mat4 ModelView;
glm::mat4 Viewport;
glm::mat4 Projection;

IShader::~IShader()
{
}

void viewport(int x, int y, int w, int h)
{
}

void projection(float coeff)
{
}

void lookat(glm::vec3 eye, glm::vec3 center, glm::vec3 up)
{
}

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>
using std::string;
using std::vector;

#define MAX_BONE_INFLUENCE 4

struct Vertex
{
	// These 3 are minimal requirements for a vertex
	// -----------------------------------------

	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;

	// -----------------------------------------

	// tangent
	glm::vec3 Tangent;
	// bitangent
	glm::vec3 Bitangent;
	//bone indexes which will influence this vertex
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float m_Weights[MAX_BONE_INFLUENCE];
};

// texture represents a diffuse or specular maps
struct Texture
{
	// unsigned int id;
	unsigned char* data;
	string type; // "diffuse" or "specular"? 
	string path; // we store the path of the texture to compare with other textures;
};

// a mesh represents a single drawable entity
class Mesh
{
public:
	// mesh Data
	vector<Vertex> vertices;
	vector<unsigned int> indices; // for indexed drawing
	vector<Texture> textures;
	unsigned int VAO;

	// ??? Should we pass these vectors as const& 
	Mesh(const vector<Vertex>& vertices, const vector<unsigned int>& indices, const vector<Texture>& textures);

	// render the mesh
	// we give a shader to the Draw function so that we can set several uniforms before drawing (like linking samplers to texture units).
	// void Draw(Shader& shader);
private:
	// render data 
	unsigned int VBO, EBO;

	// initializes all the buffer objects/arrays
	// void setupMesh();
};

#include "Model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <stb_image/stb_image.h>
#include <assimp/postprocess.h>

#include "tgaimage.h"
using std::cout;
using std::endl;


Model::Model(string const& path, bool gamma) : gammaCorrection(gamma)
{
	loadModel(path);
}

Model::~Model()
{
}


void Model::loadModel(string const& path)
{
	// read file via ASSIMP
	// -----------------------------------------

	Assimp::Importer importer;
	// use Assimp's ReadFile to load the model into a data structure called a scene object
	// ReadFile expects a file path and several post-processing options as its second argument. 
	// We want:
	// triangulate the model if the model doesn't entirely consist of triangles
	// create smooth normal vectors for each vertex if the model doesn't contain normal vectors 
	// flip the texture coordinates on the y-axis because OpenGL expects the 0.0 coordinate on the y-axis to be on the bottom side of the image, but images usually have 0.0 at the top of the y-axis
	// calculate the tangents and bitangents for the imported meshes.
	const aiScene* scene = importer.ReadFile(
		path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
		return;
	}
	// retrieve the directory path of the filepath
	directory = path.substr(0, path.find_last_of('/'));

	// process ASSIMP's root node recursively
	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	// Because each node (possibly) contains a set of children we want to first process the node in question, and then continue processing all the node's children and so on. 

	// process each mesh located at the current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}


	// Note: the exit condition of this recursive function is met when all nodes have been processed.
	// Once a node no longer has any children, the recursion stops.
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	// access each of the mesh's relevant properties and store them in our own object
	// Processing a mesh is a 3-part process:
	// 1. retrieve all the vertex data
	// 2. retrieve the mesh's indices
	// 3. retrieve the relevant material data

	// data to fill
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;

	// walk through each of the mesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		// we create a vertex from Vertex struct that will be populated and later added to the vertices vector
		Vertex vertex{};

		// Note: you can't directly assign mesh properties to glm, thus we need to assign them element by element 
		// positions
		vertex.Position.x = mesh->mVertices[i].x;
		vertex.Position.y = mesh->mVertices[i].y;
		vertex.Position.z = mesh->mVertices[i].z;

		// normals
		if (mesh->HasNormals())
		{
			vertex.Normal.x = mesh->mNormals[i].x;
			vertex.Normal.y = mesh->mNormals[i].y;
			vertex.Normal.z = mesh->mNormals[i].z;
		}

		// texture coordinates
		// Assimp allows a vertex to contain up to 8 different texture coordinates. We thus make the assumption that we won't 
		// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
			vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
			// tangent
			vertex.Tangent.x = mesh->mTangents[i].x;
			vertex.Tangent.y = mesh->mTangents[i].y;
			vertex.Tangent.z = mesh->mTangents[i].z;
			// bitangent
			vertex.Bitangent.x = mesh->mBitangents[i].x;
			vertex.Bitangent.y = mesh->mBitangents[i].y;
			vertex.Bitangent.z = mesh->mBitangents[i].z;
		}
		else
		{
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);
	}

	// Assimp defines each mesh as having an array of faces where each face represents a single primitive (due to aiProcess_Triangulate option, it's always triangle)
	// now walk through each of the mesh's faces and retrieve the corresponding vertex indices.
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		// A face contains the indices of the vertices we need to draw in what order for its primitive. 
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// process materials
	// Notice that aiMesh only contains an index to a material object (mMaterialIndex)
	// To retrieve the material of a mesh, we need to index the scene's mMaterials array.
	// A mesh only contains a single material

	// there will always be at least one material 
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
	// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
	// Same applies to other texture as the following list summarizes:
	// diffuse: texture_diffuseN
	// specular: texture_specularN
	// normal: texture_normalN

	// The different texture types are all prefixed with aiTextureType_
	// Each material may contain multiple diffuse/specular/normal/height maps

	vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

	vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

	vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

	vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

	// return a mesh object created from the extracted mesh data
	return Mesh(vertices, indices, textures);
}

vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
	vector<Texture> textures;
	// iterate through all textures of this particular type
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		// retrieve each of the texture's file locations and put it inside str
		mat->GetTexture(type, i, &str);
		// check if texture was loaded before and if so, continue to next iteration
		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++)
		{
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
				skip = true;
				// a texture with the same filepath has already been loaded, continue to next one. (optimization)
				break;
			}
		}
		if (!skip)
		{
			// if texture hasn't been loaded already, load it
			Texture texture;
			// texture.id = TextureFromFile(str.C_Str(), this->directory);
			string filename = string(str.C_Str());
			filename = directory + '/' + filename;
			cout << "texture file " << filename << " loading " << (texture.data.read_tga_file(filename)
				                                                       ? "ok"
				                                                       : "failed") << endl;
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture);
			// store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
		}
	}
	return textures;
}

// void Model::Draw(Shader& shader)
// {
// 	//  loops over each of the meshes to call their respective Draw function:
// 	for (unsigned int i = 0; i < meshes.size(); i++)
// 		meshes[i].Draw(shader);
// }


// unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
// {
// 	string filename = string(path);
// 	filename = directory + '/' + filename;
//
// 	unsigned int textureID;
// 	glGenTextures(1, &textureID);
//
// 	int width, height, nrComponents;
// 	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
// 	if (data)
// 	{
// 		GLenum format;
// 		if (nrComponents == 1)
// 			format = GL_RED;
// 		else if (nrComponents == 3)
// 			format = GL_RGB;
// 		else if (nrComponents == 4)
// 			format = GL_RGBA;
//
// 		glBindTexture(GL_TEXTURE_2D, textureID);
// 		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
// 		glGenerateMipmap(GL_TEXTURE_2D);
//
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
// 		stbi_image_free(data);
// 	}
// 	else
// 	{
// 		std::cout << "Texture failed to load at path: " << path << std::endl;
// 		stbi_image_free(data);
// 	}
//
// 	return textureID;
// }

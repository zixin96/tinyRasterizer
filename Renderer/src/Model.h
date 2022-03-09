#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <string>
#include <iostream>
#include <vector>

#include "Mesh.h"

// loads a texture (with stb_image.h) and return the texture ID
// unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

// loads a texture (with stb_image.h) and return the texture ID
unsigned char* TextureFromFile(const char* path, const string& directory, bool gamma = false);

// a model that contains multiple meshes, possibly with multiple textures. 
class Model
{
public:
	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
	vector<Texture> textures_loaded;
	vector<Mesh> meshes;
	// store the directory of the file path that we'll later need when loading textures.
	string directory;
	bool gammaCorrection;

	// constructor, expects a filepath to a 3D model.
	// It then loads the file right away via the loadModel function
	Model(string const& path, bool gamma = false);

	// responsible for freeing stb_image textures (needed for TinyOpenGL) 
	~Model();

	// draws the model, and thus all its meshes
	// void Draw(Shader& shader);

private:
	// all private functions are all designed to process a part of Assimp's import routine
	// -----------------------------------------

	// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(string const& path);

	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode* node, const aiScene* scene);

	// translates an aiMesh object to a mesh object of our own and return it 
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a vector of Texture struct.
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

	// -----------------------------------------
};

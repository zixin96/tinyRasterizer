#include "Model.h"

// use "/" for file path

// our main() function is the primitive processing routine. It calls the vertex shader. 
// We do not have primitive assembly here, since we are drawing dumb triangles only (in our code it is merged with the primitive processing). 
int main()
{
	// load models
	Model ourModel("assets/viking_room_gltf/scene.gltf");

	return 0;
}

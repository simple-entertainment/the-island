#version 330

// /////////////////////////
// Structures
// /////////////////////////

struct Surface
{
    vec4 diffuse;
	vec3 normal;
	vec3 position;
    vec4 specular;
};

struct Vertex
{
	vec4 colour;
	vec3 normal;
	vec3 position;
	vec2 texCoord;
};

// /////////////////////////
// Variables
// /////////////////////////

layout (location = 0) in Vertex vertex;

uniform mat4 cameraTransformation;
uniform mat4 worldTransformation;

out Surface surface;

// /////////////////////////
// Shader
// /////////////////////////

void main()
{
	surface.diffuse = vertex.colour;
	surface.normal = vertex.normal;

	gl_Position = cameraTransformation * worldTransformation * vec4(vertex.position, 1.0);
}

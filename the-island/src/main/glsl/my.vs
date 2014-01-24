#version 330

// /////////////////////////
// Structures
// /////////////////////////

struct VertexVS
{
	vec4 colour;
	vec3 normal;
	vec3 position;
	vec2 texCoord;
};

struct VertexFS
{
	vec3 clipPosition;
	vec4 colour;
	vec3 normal;
	vec2 texCoord;
	vec3 worldPosition;
};

// /////////////////////////
// Variables
// /////////////////////////

layout (location = 0) in VertexVS vertexVS;

uniform mat4 cameraTransformation;
uniform mat4 worldTransformation;

out VertexFS vertexFS;

// /////////////////////////
// Shader
// /////////////////////////

void main()
{
	vec4 worldPosition4 = worldTransformation * vec4(vertexVS.position, 1.0);
	vec4 clipPosition4 = cameraTransformation * worldTransformation * vec4(vertexVS.position, 1.0);

	vertexFS.clipPosition = clipPosition4.xyz;
	vertexFS.colour = vertexVS.colour;
	vertexFS.normal = vertexVS.normal;
	vertexFS.texCoord = vertexVS.texCoord;
	vertexFS.worldPosition = worldPosition4.xyz;

	// Change colours for different parts of the Island.
	if (vertexFS.colour == vec4(0.0, 0.5, 0.0, 1.0))
	{
		vec3 up = vec3(0.0, 1.0, 0.0);

		// Beaches!
		if (abs(dot(vertexFS.normal, up)) > 0.5f && vertexFS.worldPosition.y < 0.5f)
		{
			vertexFS.colour = vec4(0.83, 0.65, 0.15, 1.0);
		}

		// Cliffs!
		if (abs(dot(vertexFS.normal, up)) < 0.2f)
		{
			vertexFS.colour = vec4(0.6, 0.6, 0.6, 1.0);
		}

		// Snow!
		if (vertexFS.worldPosition.y > 20.0f)
		{
			vertexFS.colour = vec4(1.0, 1.0, 1.0, 1.0);
		}
	}

	gl_Position = clipPosition4;
}

#version 330

// /////////////////////////
// Structures
// /////////////////////////

struct Light
{
	vec4 ambient;
	vec3 attenuation;
	vec4 diffuse;
	vec3 direction;
	vec3 position;
	float range;
	vec4 specular;
	float strength;
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
// Functions
// /////////////////////////

vec4 applyDirectionalLight(VertexFS vertexFS, Light light, vec3 cameraPosition)
{
	// Add the ambient term.
	vec4 colour = vertexFS.colour * light.ambient;

	float diffuseFactor = dot(-light.direction, vertexFS.normal);
	if(diffuseFactor > 0.0f)
	{
		// Add the diffuse term.
		colour += diffuseFactor * vertexFS.colour * light.diffuse;

		// Add the specular term.
		vec3 toEye = normalize(cameraPosition - vertexFS.worldPosition);
        vec3 lightReflect = normalize(reflect(light.direction, vertexFS.normal));
        float specularFactor = dot(toEye, lightReflect);

        specularFactor = pow(specularFactor, light.strength);
        if (specularFactor > 0)
        {
            colour += specularFactor * light.specular;
        }
	}

	return colour;
}

// /////////////////////////
// Variables
// /////////////////////////

in VertexFS vertexFS;

uniform vec3 cameraPosition;
uniform Light theOnlyLight;

out vec4 colour;

// /////////////////////////
// Shader
// /////////////////////////

void main()
{
	colour = applyDirectionalLight(vertexFS, theOnlyLight, cameraPosition);
}

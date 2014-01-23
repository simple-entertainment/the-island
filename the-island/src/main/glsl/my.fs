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

struct Surface
{
    vec4 diffuse;
	vec3 normal;
	vec3 position;
    vec4 specular;
};

// /////////////////////////
// Functions
// /////////////////////////

vec4 applyDirectionalLight(Surface surface, Light light)
{
	vec4 litColour = vec4(0.0f, 0.0f, 0.0f, 1.0f);

	// Add the ambient term.
	litColour += surface.diffuse * light.ambient;        

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

	float diffuseFactor = dot(-light.direction, surface.normal);
	if(diffuseFactor > 0.0f)
	{
		//float specPower  = max(surface.specular.a, 1.0f);
		//float3 toEye     = normalize(eyePos - surface.position);
		//float3 R         = reflect(light.direction, surface.normal);
		//float specFactor = pow(max(dot(R, toEye), 0.0f), specPower);

		// diffuse and specular terms
		litColour += diffuseFactor * surface.diffuse * light.diffuse;
		//litColour += specFactor * surface.specular * light.specular;
	}

	return litColour;
}

// /////////////////////////
// Variables
// /////////////////////////

in Surface surface;

uniform Light theOnlyLight;

out vec4 finalColour;

// /////////////////////////
// Shader
// /////////////////////////

void main()
{
	finalColour = applyDirectionalLight(surface, theOnlyLight);
}

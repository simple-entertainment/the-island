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
        if (specularFactor > 0.0f)
        {
            colour += specularFactor * light.specular;
        }
	}

	return colour;
}

float getRandomFloat(vec2 seed)
{
    return fract(sin(dot(seed.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float getRandomFloatZeroToOne(vec2 seed)
{
	float random = getRandomFloat(seed);
	random = abs(random);
    return random - floor(random);
}

bool isGrain(vec3 position, float grainId, float granularity, float probability)
{
	vec2 seed = vec2(int((position.x + grainId) * granularity), int((position.z + grainId) * granularity));

    return getRandomFloatZeroToOne(seed) < probability;
}

bool near(float a, float b)
{
	return abs(a - b) < 0.0001f;
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
	VertexFS vertexFS2 = vertexFS;

	// Grass shader.
	if (near(vertexFS.colour.x, 0.0f) && near(vertexFS.colour.y, 0.5f) && near(vertexFS.colour.z, 0.0f))
	{
		vertexFS2.colour.y = getRandomFloatZeroToOne(vertexFS.worldPosition.xz) * 0.5f + 0.25f;
	}

	// Sand shader.
	if (near(vertexFS.colour.x, 0.83f) && near(vertexFS.colour.y, 0.65f) && near(vertexFS.colour.z, 0.15f))
	{
		if (isGrain(vertexFS.worldPosition.xyz, 0.0f, 100.0f, 0.3f))
		{
			vertexFS2.colour = vec4(0.72f, 0.44f, 0.04f, 1.0f);
		}
		if (isGrain(vertexFS.worldPosition.xyz, 1.0f, 100.0f, 0.3f))
		{
			vertexFS2.colour = vec4(0.6f, 0.6f, 0.6f, 1.0f);
		}
	}

	// Tree shader.
	if (near(vertexFS.colour.x, 0.47f) && near(vertexFS.colour.y, 0.24f) && near(vertexFS.colour.z, 0.0f))
	{
		vertexFS2.colour *= getRandomFloatZeroToOne(vertexFS.worldPosition.xy * 0.000001f) * 0.5f + 0.25f;
	}

	colour = applyDirectionalLight(vertexFS2, theOnlyLight, cameraPosition);
}

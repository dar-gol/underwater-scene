#version 410 core

uniform vec3 objectColor;
uniform vec3 lightDir;

in vec3 interpNormal;

float near = 0.15f;
float far = 100.0f;

float linearizeDepth(float depth)
{
	return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

float logisticDepth(float depth)
{
	// these 2 values were default function 
	// parameters before & didn't always work
	float steepness = 0.5f;
	float offset = 5.0f;

	float zVal = linearizeDepth(depth);
	return (1 / (1 + exp(-steepness * (zVal - offset))));
}

void main()
{
	float depth = logisticDepth(gl_FragCoord.z);
	vec3 normal = normalize(interpNormal);
	float diffuse = max(dot(normal, -lightDir), 0.0);
	gl_FragColor = vec4(objectColor * diffuse, 1.0) * (1.0f - depth) + vec4(depth * vec3(5.0f/255.0f, 35.0f/255.0f, 95.0f/255.0f), 1.0f);;
}

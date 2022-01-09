#version 410 core

uniform sampler2D textureSampler;
uniform vec3 lightDir;
uniform vec3 cameraPos;

in vec3 interpNormal;
in vec2 interpTexCoord;
in vec3 fragPos;

float near = 0.15f;
float far = 100.0f;

float linearizeDepth(float depth)
{
	return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

float logisticDepth(float depth, float steepness = 0.5f, float offset = 5.0f)
{
	float zVal = linearizeDepth(depth);
	return (1 / (1 + exp(-steepness * (zVal - offset))));
}


void main()
{
	float depth = logisticDepth(gl_FragCoord.z);
	vec2 modifiedTexCoord = vec2(interpTexCoord.x, 1.0 - interpTexCoord.y); // Poprawka dla tekstur Ziemi, ktore bez tego wyswietlaja sie 'do gory nogami'
	vec3 color = texture2D(textureSampler, modifiedTexCoord).rgb;

    vec3 normal = normalize(interpNormal);

	float diffuse = max(dot(normal, -lightDir), 0.0);
	gl_FragColor = vec4(color * diffuse, 1.0) * (1.0f - depth) + vec4(depth * vec3(5.0f/255.0f, 35.0f/255.0f, 95.0f/255.0f), 1.0f);
	//gl_FragColor = vec4(color * diffuse, 1.0) * (1.0f - depth) + vec4(depth * vec3(102.0f/255.0f, 172.0f/255.0f, 233.0f/255.0f), 1.0f);
	//gl_FragColor = vec4(color * diffuse, 1.0);
	//gl_FragColor = vec4(mix(color,color * diffuse+vec3(1) * specular,0.9), 1.0);
}

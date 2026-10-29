#version 450

layout(location = 0) in vec3 vPosition;
layout(location = 2) in vec2 vUV;

uniform sampler2D heightMap;
uniform float heightScale;
uniform mat4 uShadowMatrix;

void main()
{
	float height = texture(heightMap, vUV).r * heightScale;
	vec3 worldPos = vec3(vPosition.x, height, vPosition.z);

	gl_Position = uShadowMatrix * vec4(worldPos, 1.0);
}

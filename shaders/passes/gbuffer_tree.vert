#version 450

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUV;
layout(location = 6) in vec3 vColor;

layout(location = 3) in vec3 iPos;
layout(location = 4) in float iYaw;
layout(location = 5) in float iScale;

uniform mat4 modelViewProjection;

out vec3 fragNormal;
out vec3 fragColor;

void main()
{
	float c = cos(iYaw);
	float s = sin(iYaw);

	vec3 sp = vPosition * iScale;
	vec3 p = vec3(c * sp.x + s * sp.z, sp.y, -s * sp.x + c * sp.z);
	vec3 worldPos = iPos + p;

	gl_Position = modelViewProjection * vec4(worldPos, 1.0);

	fragNormal = vec3(c * vNormal.x + s * vNormal.z, vNormal.y,
					  -s * vNormal.x + c * vNormal.z);
	fragColor = vColor;
}

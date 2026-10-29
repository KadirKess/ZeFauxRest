#version 450

layout(location = 0) in vec3 vPosition;
layout(location = 3) in vec3 iPos;
layout(location = 4) in float iYaw;
layout(location = 5) in float iScale;

uniform mat4 uShadowMatrix;

void main()
{
	float c = cos(iYaw);
	float s = sin(iYaw);

	vec3 sp = vPosition * iScale;
	vec3 p = vec3(c * sp.x + s * sp.z, sp.y, -s * sp.x + c * sp.z);

	gl_Position = uShadowMatrix * vec4(iPos + p, 1.0);
}

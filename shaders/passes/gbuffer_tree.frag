#version 450

in vec3 fragNormal;
in vec3 fragColor;

layout(location = 0) out vec4 gColor;
layout(location = 1) out vec4 gNormal;

void main()
{
	// Trunk vs leaf color is baked per-vertex in Blender, so pass it through.
	// Foliage material-ID bit (gNormal.w) stays unset.
	gColor = vec4(fragColor, 1.0);
	gNormal = vec4(normalize(fragNormal), 0.0);
}

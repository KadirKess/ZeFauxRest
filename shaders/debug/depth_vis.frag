#version 450

layout(binding = 0) uniform sampler2D uDepth;
uniform vec2 uResolution;

out vec4 fragColor;

void main()
{
	float d = texture(uDepth, gl_FragCoord.xy / uResolution).r;
	fragColor = vec4(vec3(d), 1.0);
}

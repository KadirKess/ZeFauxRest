// one definition prevents horizon-band seams between sky.frag and aerial
// perspective

uniform vec3 sunDir; // points toward sun not away (same as L in lighting)

uniform vec3 hwR0, hwR1, hwR2;
uniform vec3 hwG0, hwG1, hwG2;
uniform vec3 hwB0, hwB1, hwB2;
uniform vec3 hwRad;

float _hwEval(vec3 c0, vec3 c1, vec3 c2, float cosTheta, float cosGamma)
{
	float gamma = acos(clamp(cosGamma, -1.0, 1.0));
	float expM = exp(c1.y * gamma);
	float rayM = cosGamma * cosGamma;
	float mieG = c2.z;
	float mieM = (1.0 + rayM)
		/ pow(max(1e-5, 1.0 + mieG * mieG - 2.0 * mieG * cosGamma), 1.5);
	float zenith = sqrt(max(0.0, cosTheta)); // cosTheta clamped by caller
	return (1.0 + c0.x * exp(c0.y / (cosTheta + 0.01)))
		* (c0.z + c1.x * expM + c1.z * rayM + c2.x * mieM + c2.y * zenith);
}

// below-horizon directions are clamped to avoid NaN in sqrt
vec3 sampleSky(vec3 dir)
{
	float cosTheta = clamp(dir.y, 0.0, 1.0);
	float cosGamma = clamp(dot(dir, sunDir), -1.0, 1.0);
	float r = _hwEval(hwR0, hwR1, hwR2, cosTheta, cosGamma) * hwRad.r;
	float g = _hwEval(hwG0, hwG1, hwG2, cosTheta, cosGamma) * hwRad.g;
	float b = _hwEval(hwB0, hwB1, hwB2, cosTheta, cosGamma) * hwRad.b;
	return max(vec3(0.0), vec3(r, g, b));
}

// volumetric sibling of noise.glsl, lattice wraps on integer periods to tile
// under GL_REPEAT

vec3 hash33(vec3 p)
{
	p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),
			 dot(p, vec3(269.5, 183.3, 246.1)),
			 dot(p, vec3(113.5, 271.9, 124.6)));
	return fract(sin(p) * 43758.5453123);
}

float perlin3(vec3 p, float period)
{
	vec3 i = floor(p);
	vec3 f = fract(p);
	vec3 u = f * f * (3.0 - 2.0 * f);

	float n = 0.0;
	for (int dz = 0; dz < 2; dz++)
		for (int dy = 0; dy < 2; dy++)
			for (int dx = 0; dx < 2; dx++)
			{
				vec3 o = vec3(float(dx), float(dy), float(dz));
				vec3 g = hash33(mod(i + o, period)) * 2.0 - 1.0;
				vec3 wv = mix(1.0 - u, u, o);
				n += dot(g, f - o) * wv.x * wv.y * wv.z;
			}
	return n * 0.5 + 0.5;
}

float worley3(vec3 p, float period)
{
	vec3 i = floor(p);
	vec3 f = fract(p);
	float minDist = 1.0;
	for (int dz = -1; dz <= 1; dz++)
		for (int dy = -1; dy <= 1; dy++)
			for (int dx = -1; dx <= 1; dx++)
			{
				vec3 o = vec3(float(dx), float(dy), float(dz));
				vec3 cell = mod(i + o, period);
				vec3 featurePoint = o + hash33(cell);
				minDist = min(minDist, length(featurePoint - f));
			}
	return minDist;
}

float perlinFbm3(vec3 p, float freq, int octaves)
{
	float sum = 0.0;
	float amp = 1.0;
	float total = 0.0;
	for (int i = 0; i < octaves; i++)
	{
		sum += amp * perlin3(p * freq, freq);
		total += amp;
		freq *= 2.0;
		amp *= 0.5;
	}
	return sum / total;
}

// inverted so 1 is inside a cell (a blob) not a boundary edge
float worleyFbm3(vec3 p, float freq)
{
	float w0 = 1.0 - worley3(p * freq, freq);
	float w1 = 1.0 - worley3(p * freq * 2.0, freq * 2.0);
	float w2 = 1.0 - worley3(p * freq * 4.0, freq * 4.0);
	return w0 * 0.625 + w1 * 0.25 + w2 * 0.125;
}

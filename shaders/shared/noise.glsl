vec2 hash(vec2 p)
{
	p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
	return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

float perlin(vec2 p)
{
	vec2 i = floor(p);
	vec2 f = fract(p);

	vec2 u = f * f * (3.0 - 2.0 * f);

	return mix(mix(dot(hash(i + vec2(0.0, 0.0)), f - vec2(0.0, 0.0)),
				   dot(hash(i + vec2(1.0, 0.0)), f - vec2(1.0, 0.0)), u.x),
			   mix(dot(hash(i + vec2(0.0, 1.0)), f - vec2(0.0, 1.0)),
				   dot(hash(i + vec2(1.0, 1.0)), f - vec2(1.0, 1.0)), u.x),
			   u.y);
}

float fbm(vec2 p, int octaves)
{
	float value = 0.0;
	float amplitude = 1.0;
	float frequency = 1.0;
	float totalAmplitude = 0.0;

	for (int i = 0; i < octaves; i++)
	{
		value += amplitude * perlin(p * frequency);
		totalAmplitude += amplitude;
		frequency *= 2.0;
		amplitude *= 0.5;
	}

	return value / totalAmplitude;
}

// folded and squared each octave so crests stay sharp where plain Perlin would
// smooth them
float ridged(vec2 p, int octaves)
{
	float sum = 0.0;
	float frequency = 1.0;
	float amplitude = 0.5;
	float prev = 1.0;
	float totalAmplitude = 0.0;

	for (int i = 0; i < octaves; i++)
	{
		float n = 1.0 - abs(perlin(p * frequency));
		n *= n;
		// weight by previous octave so detail only piles up on ridges
		sum += n * amplitude * prev;
		prev = n;
		totalAmplitude += amplitude;
		frequency *= 2.0;
		amplitude *= 0.5;
	}

	return sum / totalAmplitude;
}

// fbm-driven coordinate warp so straight ridges bend organically
vec2 domainWarp(vec2 p, float strength)
{
	vec2 q = vec2(fbm(p, 4), fbm(p + vec2(5.2, 1.3), 4));
	return p + strength * q;
}

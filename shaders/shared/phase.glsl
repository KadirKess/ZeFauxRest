// Cornette-Shanks normalized HG phase, g > 0 forward scatters, g < 0 back
// scatters
float hgPhase(float cosTheta, float g)
{
	float g2 = g * g;
	float denom = 1.0 + g2 - 2.0 * g * cosTheta;
	return (3.0 * (1.0 - g2) * (1.0 + cosTheta * cosTheta))
		/ (8.0 * 3.14159265 * (2.0 + g2) * pow(max(denom, 1e-4), 1.5));
}

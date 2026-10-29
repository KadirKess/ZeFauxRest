#include "app/DebugUI.hh"

#ifdef ZEFAUXREST_ENABLE_UI

#	include <cmath>

#	include "imgui.h"
#	include "imgui_impl_glfw.h"
#	include "imgui_impl_opengl3.h"
#	include "render/ColorPresets.hh"
#	include "render/StyleParams.hh"

namespace
{
	constexpr float kPi = 3.14159265358979323846f;

	// Convert the sun vector to azimuth/elevation sliders and back, so editing
	// stays on the unit sphere.
	void sunDirControls(float* dir)
	{
		float y = std::fmax(-1.0f, std::fmin(1.0f, dir[1]));
		float elevDeg = std::asin(y) * 180.0f / kPi;
		float azimDeg = std::atan2(dir[2], dir[0]) * 180.0f / kPi;

		ImGui::SliderFloat("Elevation", &elevDeg, -10.0f, 90.0f, "%.1f deg");
		ImGui::SliderFloat("Azimuth", &azimDeg, -180.0f, 180.0f, "%.1f deg");

		float elev = elevDeg * kPi / 180.0f;
		float azim = azimDeg * kPi / 180.0f;
		float cosElev = std::cos(elev);
		dir[0] = cosElev * std::cos(azim);
		dir[1] = std::sin(elev);
		dir[2] = cosElev * std::sin(azim);
	}
} // namespace

DebugUI::DebugUI(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().IniFilename =
		nullptr; // don't litter an imgui.ini next to the binary
	ImGui::StyleColorsDark();

	// install_callbacks = true chains ImGui onto the GLFW callbacks main.cpp
	// already registered, so the camera/debug keys keep firing.
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450");
	_active = true;
}

DebugUI::~DebugUI()
{
	if (!_active)
		return;
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void DebugUI::beginFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void DebugUI::buildPanel(StyleParams& s, float* sunDir)
{
	ImGui::SetNextWindowSize(ImVec2(360.0f, 640.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("Style Params");

	ImGui::Text("%.1f FPS (%.2f ms)", ImGui::GetIO().Framerate,
				1000.0f / ImGui::GetIO().Framerate);
	ImGui::TextDisabled("E toggles this panel");

	if (ImGui::Button("Reset to defaults"))
	{
		s = StyleParams{};
		float d[3] = { 0.3f, 0.6f, 0.5f };
		float len = std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
		sunDir[0] = d[0] / len;
		sunDir[1] = d[1] / len;
		sunDir[2] = d[2] / len;
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Sun & Sky", ImGuiTreeNodeFlags_DefaultOpen))
	{
		sunDirControls(sunDir);
		ImGui::ColorEdit3("Sun color", s.sunColor);
		ImGui::SliderFloat("Sun intensity", &s.sunIntensity, 0.0f, 10.0f);
		ImGui::SliderFloat("Turbidity", &s.turbidity, 1.0f, 10.0f);
		ImGui::SliderFloat("Sky radiance", &s.skyRadianceScale, 0.0f, 0.5f);
		ImGui::ColorEdit3("Sky ambient", s.skyAmbientColor);
	}

	if (ImGui::CollapsingHeader("Aerial Perspective"))
	{
		ImGui::SliderFloat("AP strength", &s.aerialPerspectiveStrength, 0.0f,
						   1.0f);
		ImGui::SliderFloat("AP start", &s.aerialPerspectiveStart, 0.0f, 200.0f);
		ImGui::SliderFloat("AP falloff", &s.aerialPerspectiveFalloff, 1.0f,
						   300.0f);
		ImGui::SliderFloat("AP canopy fade", &s.aerialPerspectiveCanopyFade,
						   0.0f, 100.0f);
	}

	if (ImGui::CollapsingHeader("Clouds"))
	{
		ImGui::ColorEdit3("Cloud ambient", s.cloudAmbientColor);
		ImGui::SliderFloat("Coverage", &s.cloudCoverage, 0.0f, 1.0f);
		ImGui::SliderFloat("Density scale", &s.cloudDensityScale, 0.0f, 4.0f);
		ImGui::SliderFloat("Detail weight", &s.cloudDetailWeight, 0.0f, 1.0f);
		ImGui::SliderFloat("Phase g", &s.cloudPhaseG, -1.0f, 1.0f);
	}

	if (ImGui::CollapsingHeader("Godrays"))
	{
		ImGui::ColorEdit3("Godray color", s.godrayColor);
		ImGui::SliderFloat("Intensity", &s.godrayIntensity, 0.0f, 20.0f);
		ImGui::SliderFloat("Phase g##godray", &s.godrayPhaseG, -1.0f, 1.0f);
		ImGui::SliderFloat("Max distance", &s.godrayMaxDistance, 0.0f, 200.0f);
		ImGui::SliderFloat("Density", &s.godrayDensity, 0.0f, 0.3f);
	}

	if (ImGui::CollapsingHeader("Grass"))
	{
		ImGui::ColorEdit3("Tip color", s.grassTipColor);
		ImGui::ColorEdit3("Base color", s.grassBaseColor);
		ImGui::SliderFloat("Hue variation", &s.grassHueVariation, 0.0f, 1.0f);
		ImGui::SliderFloat("Dryness", &s.grassDryness, 0.0f, 1.0f);
		ImGui::SliderFloat("Wind strength", &s.grassWindStrength, 0.0f, 4.0f);
		ImGui::SliderFloat("Wind gustiness", &s.grassWindGustiness, 0.0f, 2.0f);
	}

	if (ImGui::CollapsingHeader("Lighting & Shadows"))
	{
		ImGui::SliderFloat("Shadow strength", &s.shadowStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("Shadow distance", &s.shadowMaxDistance, 10.0f,
						   300.0f);
		ImGui::SliderFloat("Specular", &s.specularStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("Foliage translucency", &s.foliageTranslucency, 0.0f,
						   4.0f);
	}

	if (ImGui::CollapsingHeader("Post / Color Grade"))
	{
		// Preset stamps the grade fields
		static int presetIdx = 0;
		const char* presetNames[kColorGradePresetCount];
		for (int i = 0; i < kColorGradePresetCount; ++i)
			presetNames[i] = kColorGradePresets[i].name;
		if (ImGui::Combo("Preset", &presetIdx, presetNames,
						 kColorGradePresetCount))
			applyColorGradePreset(s, kColorGradePresets[presetIdx]);

		ImGui::SliderFloat("Bloom threshold", &s.bloomThreshold, 0.0f, 4.0f);
		ImGui::SliderFloat("Bloom strength", &s.bloomStrength, 0.0f, 2.0f);
		ImGui::DragFloat3("Lift", s.gradeLift, 0.002f, -0.5f, 0.5f);
		ImGui::DragFloat3("Gamma", s.gradeGamma, 0.005f, 0.1f, 3.0f);
		ImGui::DragFloat3("Gain", s.gradeGain, 0.005f, 0.0f, 2.0f);
		ImGui::DragFloat3("Highlight tint", s.highlightTint, 0.005f, 0.0f,
						  2.0f);
		ImGui::DragFloat3("Shadow tint", s.shadowTint, 0.002f, -0.5f, 0.5f);
		ImGui::SliderFloat("Vignette", &s.vignetteStrength, 0.0f, 1.0f);
	}

	ImGui::End();
}

void DebugUI::render()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

#else // ZEFAUXREST_ENABLE_UI

DebugUI::DebugUI(GLFWwindow*)
{}
DebugUI::~DebugUI()
{}
void DebugUI::beginFrame()
{}
void DebugUI::buildPanel(StyleParams&, float*)
{}
void DebugUI::render()
{}

#endif

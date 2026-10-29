#pragma once

struct GLFWwindow;
struct StyleParams;

// Compiled to no-ops unless ZEFAUXREST_ENABLE_UI is defined.
class DebugUI
{
private:
	bool _active = false;

public:
	explicit DebugUI(GLFWwindow* window);
	~DebugUI();

	DebugUI(const DebugUI&) = delete;
	DebugUI& operator=(const DebugUI&) = delete;

	void beginFrame();
	void buildPanel(StyleParams& style, float* sunDir);
	void render();
};

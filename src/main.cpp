// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
// clang-format on

#include "app/Camera.hh"
#include "app/DebugUI.hh"
#include "gl/Shader.hh"
#include "helpers/Frustum.hh"
#include "render/RenderGraph.hh"

static constexpr float NEAR_PLANE = 0.1f;
static constexpr float FAR_PLANE = 500.0f;
static constexpr float FOV_Y = 60.0f;

static bool screenshotRequested = false;
static int debugVisMode = 0; // 0 = final, 1-9 = pass debug blit
static bool uiVisible = false;

static void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id,
									 GLenum severity, GLsizei /*length*/,
									 const GLchar* message,
									 const void* /*userParam*/)
{
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;

	const char* srcStr = "?";
	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		srcStr = "API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		srcStr = "Window";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		srcStr = "Shader";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		srcStr = "3rdParty";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		srcStr = "App";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		srcStr = "Other";
		break;
	}

	const char* typeStr = "?";
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		typeStr = "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		typeStr = "Deprecated";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		typeStr = "Undefined";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		typeStr = "Portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		typeStr = "Perf";
		break;
	case GL_DEBUG_TYPE_MARKER:
		typeStr = "Marker";
		break;
	case GL_DEBUG_TYPE_OTHER:
		typeStr = "Other";
		break;
	}

	const char* sevStr = "?";
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		sevStr = "HIGH";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		sevStr = "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_LOW:
		sevStr = "LOW";
		break;
	}

	std::fprintf(stderr, "[GL %s][%s][%s] %s\n", sevStr, srcStr, typeStr,
				 message);
}

static void keyCallback(GLFWwindow* window, int key, int /*scancode*/,
						int action, int /*mods*/)
{
	if (action != GLFW_PRESS)
		return;

	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (key == GLFW_KEY_P)
		screenshotRequested = true;

	if (key == GLFW_KEY_E)
		uiVisible = !uiVisible;

	if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
	{
		debugVisMode = key - GLFW_KEY_0;
		std::printf("Debug vis: %d\n", debugVisMode);
	}
}

static void framebufferSizeCallback(GLFWwindow* /*window*/, int width,
									int height)
{
	glViewport(0, 0, width, height);
}

static void saveScreenshot(int width, int height)
{
	size_t size = (size_t)width * height * 3;
	unsigned char* pixels = new unsigned char[size];
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	FILE* f = std::fopen("screenshot.ppm", "wb");
	if (f)
	{
		std::fprintf(f, "P6\n%d %d\n255\n", width, height);
		// flip vertically (OpenGL bottom-left origin)
		for (int y = height - 1; y >= 0; --y)
			std::fwrite(pixels + y * width * 3, 1, (size_t)width * 3, f);
		std::fclose(f);
		std::printf("Screenshot saved: screenshot.ppm\n");
	}

	delete[] pixels;
}

static void blitColorAttachment(GLuint fboId, GLenum attachment, int width,
								int height)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
	glReadBuffer(attachment);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
					  GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static Matrix4 buildProjection(float fovYDeg, float aspect, float near,
							   float far)
{
	float tanHalf = std::tan(fovYDeg * 3.14159265f / 360.0f);
	float top = near * tanHalf;
	float right = top * aspect;
	return frustum(-right, right, -top, top, near, far);
}

int main()
{
	if (!glfwInit())
	{
		std::fprintf(stderr, "Failed to init GLFW\n");
		return EXIT_FAILURE;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	int winW = mode->width;
	int winH = mode->height;

	GLFWwindow* window =
		glfwCreateWindow(winW, winH, "ZeFauxRest", monitor, nullptr);
	if (!window)
	{
		std::fprintf(stderr, "Failed to create GLFW window\n");
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::fprintf(stderr, "Failed to init GLAD\n");
		glfwTerminate();
		return EXIT_FAILURE;
	}

	std::printf("OpenGL %s | %s | %s\n", glGetString(GL_VERSION),
				glGetString(GL_RENDERER), glGetString(GL_VENDOR));

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(glDebugCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
						  GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	{
		Camera camera(0.0f, 20.0f, 0.0f);
		RenderGraph renderGraph(winW, winH);
		DebugUI ui(window);
		bool cursorFree = false; // mirrors uiVisible, tracks GLFW cursor mode

		float aspect = (float)winW / (float)winH;
		Matrix4 projection =
			buildProjection(FOV_Y, aspect, NEAR_PLANE, FAR_PLANE);

		// Reusable read FBO for blitting standalone textures to screen,
		// attachment rebound per use.
		GLuint debugReadFbo = 0;
		glCreateFramebuffers(1, &debugReadFbo);
		glObjectLabel(GL_FRAMEBUFFER, debugReadFbo, -1, "debugReadFbo");

		// Depth textures can't be color-blitted, so sample them to grayscale
		// with a fullscreen triangle instead.
		Shader depthVis(SHADER_DIR "/debug/fullscreen.vert",
						SHADER_DIR "/debug/depth_vis.frag");
		GLuint emptyVao = 0;
		glCreateVertexArrays(1, &emptyVao);
		glObjectLabel(GL_VERTEX_ARRAY, emptyVao, -1, "debugEmptyVAO");

		float lastTime = (float)glfwGetTime();

		while (!glfwWindowShouldClose(window))
		{
			float currentTime = (float)glfwGetTime();
			float dt = currentTime - lastTime;
			lastTime = currentTime;

			glfwPollEvents();

			// Hand the cursor between camera and UI. Reset the camera's mouse
			// delta on re-grab so the view doesn't snap.
			if (uiVisible != cursorFree)
			{
				glfwSetInputMode(window, GLFW_CURSOR,
								 uiVisible ? GLFW_CURSOR_NORMAL
										   : GLFW_CURSOR_DISABLED);
				if (!uiVisible)
					camera.resetMouseState();
				cursorFree = uiVisible;
			}

			if (!uiVisible)
				camera.update(window, dt);

			if (uiVisible)
			{
				ui.beginFrame();
				ui.buildPanel(renderGraph.style(), renderGraph.sunDir());
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			renderGraph.execute(camera.getViewMatrix(), projection.m,
								currentTime);

			// Debug blit canonical numbering
			// 1 gAlbedo
			// 2 gNormal
			// 3 gDepth
			// 4 shadowMap
			// 5 litScene
			// 6 cloudRGBA
			// 7 godrayRGB
			// 8 hdrComposite
			// (0 = final)
			if (debugVisMode > 0)
			{
				GLuint gBufferFbo = renderGraph.gbuffer().fboId();

				auto blitDepthTexture = [&](const Texture& tex) {
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					glViewport(0, 0, winW, winH);
					depthVis.use();
					tex.bindToUnit(0);
					float res[2] = { (float)winW, (float)winH };
					depthVis.setUniformVec2("uResolution", res);
					glDisable(GL_DEPTH_TEST);
					glBindVertexArray(emptyVao);
					glDrawArrays(GL_TRIANGLES, 0, 3);
					glEnable(GL_DEPTH_TEST);
				};

				auto blitColorTexture = [&](const Texture& tex) {
					glNamedFramebufferTexture(
						debugReadFbo, GL_COLOR_ATTACHMENT0, tex.id(), 0);
					blitColorAttachment(debugReadFbo, GL_COLOR_ATTACHMENT0,
										winW, winH);
				};

				switch (debugVisMode)
				{
				case 1:
					blitColorAttachment(gBufferFbo, GL_COLOR_ATTACHMENT0, winW,
										winH);
					break;
				case 2:
					blitColorAttachment(gBufferFbo, GL_COLOR_ATTACHMENT1, winW,
										winH);
					break;
				case 3:
					blitDepthTexture(renderGraph.gbuffer().depth());
					break;
				case 4:
					blitDepthTexture(renderGraph.shadowMap());
					break;
				case 5:
					blitColorTexture(renderGraph.litScene());
					break;
				case 6:
					// half-res, blitColorTexture rescales via GL_LINEAR
					blitColorTexture(renderGraph.cloudRGBA());
					break;
				case 7:
					blitColorTexture(renderGraph.godrayRGB());
					break;
				case 8:
					blitColorTexture(renderGraph.hdrComposite());
					break;
				default:
					break;
				}
			}

			if (screenshotRequested)
			{
				int w, h;
				glfwGetFramebufferSize(window, &w, &h);
				saveScreenshot(w, h);
				screenshotRequested = false;
			}

			if (uiVisible)
				ui.render();

			glfwSwapBuffers(window);
		}

		glDeleteFramebuffers(1, &debugReadFbo);
		glDeleteVertexArrays(1, &emptyVao);
	} // GL objects destroyed here, before context teardown

	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}

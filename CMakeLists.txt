cmake_minimum_required(VERSION 3.16)
project(ZeFauxRest LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_compile_definitions(_USE_MATH_DEFINES)
add_library(glad STATIC third_party/glad/src/glad.c)
target_include_directories(glad PUBLIC third_party/glad/include)

file(GLOB MESHOPT_SOURCES third_party/meshoptimizer/src/*.cpp)
add_library(meshoptimizer STATIC ${MESHOPT_SOURCES})
target_include_directories(meshoptimizer PUBLIC third_party/meshoptimizer/src)

option(ZEFAUXREST_ENABLE_UI "Build the Dear ImGui debug/tuning panel" ON)
if(ZEFAUXREST_ENABLE_UI)
	add_library(imgui STATIC
		third_party/imgui/imgui.cpp
		third_party/imgui/imgui_draw.cpp
		third_party/imgui/imgui_tables.cpp
		third_party/imgui/imgui_widgets.cpp
		third_party/imgui/backends/imgui_impl_glfw.cpp
		third_party/imgui/backends/imgui_impl_opengl3.cpp)
	target_include_directories(imgui PUBLIC third_party/imgui third_party/imgui/backends)
	target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE include third_party/tinyobjloader third_party/hosek_wilkie third_party/meshoptimizer/src)
target_link_libraries(${PROJECT_NAME} PRIVATE glad glfw OpenGL::GL meshoptimizer)

if(ZEFAUXREST_ENABLE_UI)
	target_link_libraries(${PROJECT_NAME} PRIVATE imgui)
	target_compile_definitions(${PROJECT_NAME} PRIVATE ZEFAUXREST_ENABLE_UI)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
	SHADER_DIR="${CMAKE_SOURCE_DIR}/shaders"
	ASSET_DIR="${CMAKE_SOURCE_DIR}/assets")

add_custom_target(run
	COMMAND ${CMAKE_COMMAND} -E env
		__NV_PRIME_GPU_OFFLOADING=1
		__GLX_VENDOR_LIBRARY_NAME=nvidia
		$<TARGET_FILE:${PROJECT_NAME}>
	DEPENDS ${PROJECT_NAME}
)

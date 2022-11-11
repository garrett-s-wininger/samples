bin_PROGRAMS = initial_primitive
initial_primitive_SOURCES = src/main.cpp
initial_primitive_CXXFLAGS = -std=c++17
BUILT_SOURCES = vert.spv frag.spv

vert.spv: src/shaders/shader.vert
	glslc src/shaders/shader.vert -o vert.spv

frag.spv: src/shaders/shader.frag
	glslc src/shaders/shader.frag -o frag.spv

clean-local:
	rm -f *.spv

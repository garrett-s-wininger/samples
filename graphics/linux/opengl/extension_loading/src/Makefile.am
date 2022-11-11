bin_PROGRAMS = extension_loading
extension_loading_CPPFLAGS = -Igenerated/include
extension_loading_SOURCES = main.cpp generated/src/glad.c
BUILT_SOURCES = generated/src/glad.c

generated/src/glad.c:
	glad --profile core --out-path generated --generator c

clean-local:
	rm -rf generated/


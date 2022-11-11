#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

static void error_callback(int error, const char* description)
{
	std::cerr << "Error: " << description << "\n";
}

int main (int argc, char** argv)
{
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	GLFWwindow* window = glfwCreateWindow(640, 480, "Context", nullptr, nullptr);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	
	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
	{
		exit(EXIT_FAILURE);
	}

	std::cout << "OpenGL Version: " << GLVersion.major << "." << GLVersion.minor << "\n";

	while (!glfwWindowShouldClose(window))
	{
		int width, height;

		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}


AC_INIT([extension_listing], [2022.11.11], [garrett.s.wininger@outlook.com])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_FILES([Makefile])
AC_PROG_CXX
AC_CHECK_LIB([glfw], [glfwInit], [], [AC_MSG_ERROR([GLFW Unavailable])])
AC_CHECK_HEADER([GLFW/glfw3.h], [], [AC_MSG_ERROR([GLFW Unavailable])])
AC_CHECK_LIB([vulkan], [vkEnumerateInstanceExtensionProperties], [], [AC_MSG_ERROR([Vulkan Unavailable])])
AC_OUTPUT

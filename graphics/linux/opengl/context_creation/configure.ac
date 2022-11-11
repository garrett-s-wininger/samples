AC_INIT([context_creation], [2022.10.30], [garrett.s.wininger@outlook.com])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile src/Makefile])
AC_PROG_CXX
AC_CHECK_LIB([GL], [glViewport], [], [AC_MSG_ERROR([OpenGL Unavailable])])
AC_CHECK_HEADER([GL/gl.h], [], [AC_MSG_ERROR([OpenGL Unavailable])])
AC_CHECK_LIB([glfw], [glfwInit], [],  [AC_MSG_ERROR([GLFW Unavailable])])
AC_CHECK_HEADER([GLFW/glfw3.h], [], [AC_MSG_ERROR([GLFW Unavailable])])
AC_OUTPUT

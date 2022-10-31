AC_INIT([extension_loading], [2022.10.30], [garrett.s.wininger@outlook.com])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_PROG_CXX
AC_CHECK_PROG([glad], [glad], [yes], [no])
AS_IF([test "$glad" = "yes"], [], [AC_MSG_ERROR([GLAD Unavailable])])
AC_CHECK_LIB([glfw], [glfwInit], [],  [AC_MSG_ERROR([GLFW Unavailable])])
AC_CHECK_HEADER([GLFW/glfw3.h], [], [AC_MSG_ERROR([GLFW Unavailable])])
AC_CHECK_LIB([dl], [dlclose], [], [AC_MSG_ERROR([DL Unavailable])])
AC_OUTPUT

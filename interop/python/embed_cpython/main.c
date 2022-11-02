#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdbool.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	FILE* script = fopen("main.py", "r");

	if (script == NULL)
	{
		exit(1);
	}

	Py_Initialize();
	PyRun_SimpleFileEx(script, "main.py", true);

	if (Py_FinalizeEx() < 0)
	{
		exit(120);
	}

	return 0;
}

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdbool.h>
#include <stdio.h>

static PyObject* embed_say_hello(PyObject* self, PyObject* args)
{
	printf("Hello from Embedded Python Extension!\n");
	Py_RETURN_NONE;
}

static PyMethodDef EmbedMethods[] = {
	{"say_hello", embed_say_hello, METH_VARARGS, "Says hello"},
	{NULL, NULL, 0, NULL}
};

static PyModuleDef EmbedModule = {
	PyModuleDef_HEAD_INIT, "embed", NULL, -1, EmbedMethods,
	NULL, NULL, NULL, NULL
};

static PyObject* PyInit_embed(void)
{
	return PyModule_Create(&EmbedModule);
}

int main(int argc, char** argv)
{
	FILE* script = fopen("main.py", "r");

	if (script == NULL)
	{
		exit(1);
	}


	PyImport_AppendInittab("embed", &PyInit_embed);
	Py_Initialize();
	PyRun_SimpleFileEx(script, "main.py", true);

	if (Py_FinalizeEx() < 0)
	{
		exit(120);
	}

	return 0;
}

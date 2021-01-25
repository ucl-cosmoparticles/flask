#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <string.h>

extern int flask(int, char*[]);


static PyObject* flask_main(PyObject* self, PyObject* args)
{
    PyObject* list;
    PyObject* item;
    const char* str;
    Py_ssize_t len;
    int argc, i, err;
    char** argv;

    if(!PyArg_ParseTuple(args, "O!", &PyList_Type, &list))
        return NULL;

    argc = PyList_Size(list);
    argv = (char**)malloc(argc*sizeof(char*));
    if(!argv)
        return PyErr_NoMemory();

    for(i = 0; i < argc; ++i)
    {
        item = PyList_GetItem(list, i);
        str = PyUnicode_AsUTF8AndSize(item, &len);
        if(!str)
            break;
        argv[i] = (char*)malloc(len+1);
        strncpy(argv[i], str, len+1);
    }

    if(i < argc)
    {
        free(argv);
        return NULL;
    }

    err = flask(argc, argv);

    return PyLong_FromLong(err);
}


static PyMethodDef methods[] = {
    {"flask", flask_main, METH_VARARGS, PyDoc_STR(
        "flask(args)\n"
        "--\n"
        "\n"
        "Parameters\n"
        "----------\n"
        "args : list of str\n"
        "    List of command line options. First entry must be \"flask\".\n"
        "\n"
        "Returns\n"
        "-------\n"
        "status : int\n"
        "    The return value from Flask.\n"
    )},
    {NULL, NULL}
};


static struct PyModuleDef module = {
    PyModuleDef_HEAD_INIT,
    "pyFlask",
    PyDoc_STR("Flask Python wrapper"),
    -1,
    methods
};


PyMODINIT_FUNC
PyInit_pyFlask(void)
{
    return PyModule_Create(&module);
}

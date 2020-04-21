#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <time.h>

PyObject *time_ex_time(PyObject *self, PyObject *unsed) {
  return PyLong_FromLong(time(NULL));
}

static PyMethodDef time_ex_methods[] = {
    {
        .ml_name = "time",          // 函数名 const char *
        .ml_meth = time_ex_time,    // C 函数指针 PyCFunction
        .ml_flags = METH_NOARGS,    // 调用参数标志 int
        .ml_doc = "返回时间戳秒数"  // 函数的文档 const char*
    },
    {NULL, NULL, 0, NULL}, /* Sentinel*/
};

static struct PyModuleDef time_ex_module = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "time_ex",                  // 模块名
    .m_doc = "一个模仿 time 模块的模块",  // 模块文档说明
    .m_size = -1,  // 解释器实例为此模块分配的私有内存大小. -1 表示没有.
    .m_methods = time_ex_methods,  //  模块函数列表
};

PyMODINIT_FUNC PyInit_time_ex(void) {
  return PyModule_Create(&time_ex_module);
}
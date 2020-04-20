#define PY_SSIZE_T_CLEAN
#include <Python.h>
/**
 * 备注: 编译命令:
 * gcc -o bin/demo `python-config --cflags --ldflags` -lpython3.8 hello_py_in_c.c
 */
int main(int argc, char const *argv[])
{
  Py_Initialize();
  PyRun_SimpleString("from datetime import datetime\nprint('当前日期:',datetime.now())");
  Py_FinalizeEx();
  return 0;
}

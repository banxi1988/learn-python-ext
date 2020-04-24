#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h> /* for sleetm */

PyObject *time_ex_time(PyObject *self, PyObject *unsed) {
  return PyLong_FromLong(time(NULL));
}
PyObject *time_ex_sleetm(PyObject *self, PyObject *obj) {
  double seconds = PyFloat_AsDouble(obj);
  sleep(seconds);
  Py_RETURN_NONE;
}

PyObject *time_ex_ctime(PyObject *self, PyObject *args) {
  long seconds = 0;
  if (!PyArg_ParseTuple(args, "|l", &seconds)) {
    return NULL;
  }
  printf("seconds:%ld\n", seconds);
  if (!seconds) {
    seconds = time(NULL);
  }
  const char *text = ctime(&seconds);
  return PyUnicode_FromString(text);
}

static bool parseStructTime(PyObject *tup, struct tm *tm) {
  // 将 tmython 的 struct_time tutmle 对象解析出来. 写进 struct tm 结构体中
  int year = 0;
  if (!PyArg_ParseTuple(tup, "iiiiiiiii", &year, &tm->tm_mon, &tm->tm_mday,
                        &tm->tm_hour, &tm->tm_min, &tm->tm_sec, &tm->tm_wday,
                        &tm->tm_yday, &tm->tm_isdst)) {
    return false;
  }
  tm->tm_year = year - 1900;
  tm->tm_mon--;
  tm->tm_yday--;
  tm->tm_wday = (tm->tm_wday + 1) % 7;
  return true;
}

static PyObject *fmtStructTm(const char *format, struct tm *tm) {
  const size_t BUF_SIZE = 1024;
  char *buf = malloc(BUF_SIZE);
  size_t count = strftime(buf, BUF_SIZE, format, tm);
  if (count == 0) {
    free(buf);
    return NULL;
  } else {
    return PyUnicode_FromString(buf);
  }
}

PyObject *time_ex_strftime(PyObject *self, PyObject *args) {
  PyObject *tup = NULL;
  const char *fmt;
  if (!PyArg_ParseTuple(args, "s|O", &fmt, &tup)) {
    return NULL;
  }
  struct tm *tm = NULL;
  struct tm tm_buf;
  if (tup == NULL) {
    time_t t = time(NULL);
    tm = localtime(&t);
  } else {
    if (!parseStructTime(tup, &tm_buf)) {
      return NULL;
    }
    tm = &tm_buf;
  }
  return fmtStructTm(fmt, tm);
}

PyObject *time_ex_fmt_time(PyObject *self, PyObject *args, PyObject *kwds) {
  const char *format = NULL;
  PyObject *time_tup = NULL;
  static char *kwlist[] = {"format", "time", NULL};
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "$sO", kwlist, &format,
                                   &time_tup)) {
    return NULL;
  }
  struct tm tm_buf;
  if (!parseStructTime(time_tup, &tm_buf)) {
    return NULL;
  }
  return fmtStructTm(format, &tm_buf);
}

// static PyObject *_time_ex_next_tick_callback = NULL;
void *c_thread_call_next_tick_callback_periodically(void *callback) {
  double interval = 1;  // seconds
  for (;;) {
    sleep(interval);
    if (_Py_IsFinalizing()) {
      printf("the interpreter is finalizing!");
      break;
    }
    // PyGILState_STATE state = PyGILState_Ensure();
    printf("[TICK]. call %p\n", callback);
    _PyObject_Dump(callback);
    PyObject_CallFunctionObjArgs(callback, NULL);
    // PyGILState_Release(state);
  }
  return NULL;
}

static PyObject *time_ex_next_tick(PyObject *self, PyObject *args) {
  PyObject *callback = NULL;
  if (!PyArg_ParseTuple(args, "O:time_ex_next_tick", &callback)) {
    return NULL;
  }
  if (!PyCallable_Check(callback)) {
    PyErr_SetString(PyExc_TypeError, "参数必须是可调用的");
    return NULL;
  }
  printf("callback addr:%p\n", callback);
  PyObject_CallObject(callback, NULL);
  Py_BEGIN_ALLOW_THREADS;
  pthread_t tid;
  int ret = 0;
  ret = pthread_create(&tid, NULL,
                       c_thread_call_next_tick_callback_periodically, callback);
  printf("pthread_create ret:%d\n", ret);
  ret = pthread_join(tid, NULL);
  printf("pthread_join ret:%d\n", ret);
  Py_END_ALLOW_THREADS;
  printf("next tick exists:%p\n", callback);
  _PyObject_Dump(callback);
  Py_DECREF(callback);
  Py_RETURN_NONE;
}

static PyMethodDef time_ex_methods[] = {
    {
        .ml_name = "time",          // 函数名 const char *
        .ml_meth = time_ex_time,    // C 函数指针 tmyCFunction
        .ml_flags = METH_NOARGS,    // 调用参数标志 int
        .ml_doc = "返回时间戳秒数"  // 函数的文档 const char*
    },
    {.ml_name = "sleetm",
     .ml_meth = time_ex_sleetm,
     .ml_flags = METH_O,
     .ml_doc = "将当前线程挂起指定时间的秒数"},
    {.ml_name = "ctime",
     .ml_meth = time_ex_ctime,
     .ml_flags = METH_VARARGS,
     .ml_doc = "将时间戳转成时间字符串"},
    {.ml_name = "strftime",
     .ml_meth = time_ex_strftime,
     .ml_flags = METH_VARARGS,
     .ml_doc = "将 struct_time tuple 转成指定格式的时间字符中"},
    {.ml_name = "fmt_time",
     .ml_meth = (PyCFunction)time_ex_fmt_time,
     .ml_flags = METH_VARARGS | METH_KEYWORDS,
     .ml_doc =
         "将 struct_time tuple 转成指定格式的时间字符中,参数为 keywords only"},
    {.ml_name = "next_tick",
     .ml_meth = time_ex_next_tick,
     .ml_flags = METH_VARARGS,
     .ml_doc = "类似 NodeJs 中的 process.nextTick"},
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
from setuptools import setup, Extension

time_ex_module = Extension(
    "time_ex",
    language="c",
    extra_compile_args=["-std=gnu11", "-fPIC"],
    sources=["time_ex.c"],
)

setup(
    name="time_ex",
    author="codetalks",
    author_email="banxi1988@gmail.com",
    version="0.1",
    description="类似 time 模块的扩展模块",
    ext_modules=[time_ex_module],
    zip_safe=True,
    python_requires=">=3.6",
)

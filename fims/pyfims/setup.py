from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

fims_extension = Extension(
    name="pyfims",
    sources=["pyfims.pyx"],
    libraries=["fims"],
    library_dirs=["/usr/local/lib","../build/release"],
    include_dirs=["/usr/local/include/fims","../include","../src"],
    language="c++",
    extra_link_args=["-Wl,-rpath,/usr/local/lib"]
)

setup(
    ext_modules=cythonize(fims_extension,compiler_directives={'language_level':3}),
    name="pyfims",
    version="1.3.0",
    description="Python FIMS Module"
)
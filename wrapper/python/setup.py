#---------------------------------------------------------------------------

from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

extensions = [
    Extension("swif", ["swif.pyx"],
        include_dirs=["../../src"],
        libraries=["swif"],
        library_dirs=["../../src"]),
]

module = cythonize("swif.pyx")

setup(name='SWIF Codec', ext_modules=cythonize(extensions))

#---------------------------------------------------------------------------

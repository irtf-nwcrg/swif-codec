#---------------------------------------------------------------------------

from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

extensions = [
    Extension("swif", ["swif.pyx"],
        include_dirs=["../../src", "../../liblc/src"],
        libraries=["swif", "lc"],
        #libraries=["swif"],
        library_dirs=["../../src", "../../liblc/src"]),
]

module = cythonize("swif.pyx", language_level=3)

setup(name='SWIF Codec', ext_modules=cythonize(extensions))

#---------------------------------------------------------------------------

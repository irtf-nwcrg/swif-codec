#---------------------------------------------------------------------------
# swif-codec - 2019
#---------------------------------------------------------------------------

LIBLC_URL=https://github.com/gardinet/liblc

all: ensure-liblc compile-libswif
	python3 setup.py build_ext --inplace
	test -e swif.so || ln -s swif.*.so swif.so

ensure-liblc:
	test -e ../../liblc || make ../../liblc
	test -e ../../liblc/liblc.a || (cd ../../liblc/src && make)

ensure-libswif:
	test -e ../../src/libswif.a || (cd ../../src && make)

compile-libswif:
	cd ../../src && make

../../liblc:
	cd ../.. && git clone ${LIBLC_URL}

recompile: ensure-liblc
	(cd ../../src && make clean && make)
	(cd ../../liblc/src && make clean && make)
	make clean && make

clean:
	rm -rf *~ swif.c swif.*.so swif.so build

test:
	python3 -m pytest

#---------------------------------------------------------------------------

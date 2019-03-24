#---------------------------------------------------------------------------
# C.A. - 2019
#---------------------------------------------------------------------------

LIBLC_URL=https://github.com/gardinet/liblc

all: ensure-liblc
	python3 setup.py build_ext --inplace
	test -e swif.so || ln -s swif.*.so swif.so

ensure-liblc:
	test -e ../../libc || make ../../liblc
	test -e ../../libc/liblc.a || (cd ../../liblc/src && make)

../../liblc:
	cd ../.. && git clone ${LIBLC_URL}

clean:
	rm -rf *~ swif.c swif.*.so swif.so build

#---------------------------------------------------------------------------

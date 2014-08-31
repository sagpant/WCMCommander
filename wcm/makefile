
ifeq ($(OS),Windows_NT)
include makefile.windows
else
all: libconf_ux.h wcm__

wcm__: 
	make -f makefile.int
	cat info.txt

libconf_ux.h: libtester/libconf.create
	cd libtester &&	./libconf.create
	
clean:
	rm -f libconf_ux.h
	make -f makefile.int clean

install:
	make -f makefile.int install
endif

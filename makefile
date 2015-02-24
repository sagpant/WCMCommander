ifeq ($(OS),Windows_NT)
include src/makefile.windows
else
include src/makefile.linux
endif

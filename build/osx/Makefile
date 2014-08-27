# Makefile

DIR_MAKE = $(shell pwd)
DIR_HOME = $(DIR_MAKE)/../..
DIR_BIN = $(DIR_HOME)/bin/osx

export DIR_HOME
export DIR_OUTPUT_ZLIB = $(DIR_MAKE)/_build/zlib
export DIR_OUTPUT_LIBPNG = $(DIR_MAKE)/_build/libpng
export DIR_OUTPUT_LUA = $(DIR_MAKE)/_build/lua
export DIR_OUTPUT_SIMPLEPACKER = $(DIR_MAKE)/_build/simplepacker

all: simplepacker

simplepacker: lua libpng
	-@if [ ! -d $(DIR_OUTPUT_SIMPLEPACKER) ]; then mkdir -p $(DIR_OUTPUT_SIMPLEPACKER); fi
	make -C $(DIR_HOME)/src/ -f $(DIR_MAKE)/Makefile-simplepacker
	cp $(DIR_HOME)/src/scripts/*.lua $(DIR_OUTPUT_SIMPLEPACKER)
	@echo [Build simplepacker finish]

lua:
	-@if [ ! -d $(DIR_OUTPUT_LUA) ]; then mkdir -p $(DIR_OUTPUT_LUA); fi
	make -C $(DIR_HOME)/3rd/lua/ -f $(DIR_MAKE)/Makefile-lua

libpng: zlib
	-@if [ ! -d $(DIR_OUTPUT_LIBPNG) ]; then mkdir -p $(DIR_OUTPUT_LIBPNG); fi
	make -C $(DIR_HOME)/3rd/libpng/ -f $(DIR_MAKE)/Makefile-libpng

zlib:
	-@if [ ! -d $(DIR_OUTPUT_ZLIB) ]; then mkdir -p $(DIR_OUTPUT_ZLIB); fi
	make -C $(DIR_HOME)/3rd/zlib/ -f $(DIR_MAKE)/Makefile-zlib

test:
	cd $(DIR_OUTPUT_SIMPLEPACKER); \
		./simplepacker $(DIR_HOME)/test/input -o $(DIR_HOME)/test/output -ps 512 -v; \
		./simplepacker $(DIR_HOME)/test/input_advance -o $(DIR_HOME)/test/output -v
	@echo [Run tests finish]

clean:
	rm -rf $(DIR_MAKE)/_build

.PHONY: all clean test simplepacker lua libpng zlib

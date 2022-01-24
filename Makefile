EMACS_ROOT ?= ../..
EMACS ?= emacs
LUA_VERSION ?= 5.2

CC      = gcc
LD      = gcc
CPPFLAGS = -I$(EMACS_ROOT)/src $(shell pkg-config --cflags lua-$(LUA_VERSION))
CFLAGS = -std=gnu99 -ggdb3 -Wall -fPIC $(CPPFLAGS)
LUA_LIBS = $(shell pkg-config --libs lua-$(LUA_VERSION))

.PHONY : test

all: lua-core.so

lua-core.so: lua-core.o
	$(LD) -shared $(LDFLAGS) -o $@ $^ $(LUA_LIBS)

lua-core.o: lua-core.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm -f lua-core.so lua-core.o

test: lua-core.so
	$(EMACS) -Q -batch -L . $(LOADPATH) \
		-l test/test.el \
		-f ert-run-tests-batch-and-exit

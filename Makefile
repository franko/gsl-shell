
# Makefile
# 
# Copyright (C) 2009 Francesco Abbate
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#

include makeconfig

ifeq ($(target),mingw)
# Option for Windows Platform
  DEFS += -DWIN32
  INCLUDES += -I/usr/include
  LIBS += -L/usr/lib
else
  INCLUDES += -DLUA_USE_LINUX
endif

ifeq ($(target),mingw)
  TARGETS = gsl-shell.exe
else
  TARGETS = gsl-shell
endif

LIBTOOL = libtool --silent --tag=CC
LUADIR = lua-5.1.4
AR= ar rcu
RANLIB= ranlib

ifeq ($(target),mingw)
  LUA_CFLAGS =
  LUA_LIBS = -L$(LUADIR)/src -llua51
else
  LUA_CFLAGS := $(shell pkg-config lua5.1 --cflags)
  LUA_LIBTOOL := $(shell pkg-config lua5.1 --variable=libtool_lib)
  RPATH=$(shell pkg-config lua5.1 --define-variable=prefix=/usr/local \
	--variable=INSTALL_CMOD)
endif

COMPILE = $(CC) $(CFLAGS) $(DEFS) $(INCLUDES)

LUAGSL_SRC_FILES = math-types.c matrix.c cmatrix.c nlinfit_helper.c \
		fdfsolver.c nlinfit.c cnlinfit.c lua-utils.c linalg.c \
		integ.c ode_solver.c ode.c lua-gsl.c

SUBDIRS = 

LUAGSL_OBJ_FILES := $(LUAGSL_SRC_FILES:%.c=%.o)
LUAGSL_LOBJ_FILES := $(LUAGSL_SRC_FILES:%.c=%.lo)

DEP_FILES := $(LUAGSL_SRC_FILES:%.c=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

GSL_LIBS = -lgsl -lgslcblas -lm

all: $(TARGETS)

ifeq ($(target),mingw)

gsl-shell.o: gsl-shell.c
	$(CC) -c --std=c99 $(CFLAGS) $(DEFS) -I$(LUADIR)/src $(INCLUDES) $(LUA_CFLAGS) $<

gsl-shell.exe: $(LUAGSL_OBJ_FILES) gsl-shell.o
	$(CC) -o $@ gsl-shell.o $(LUAGSL_OBJ_FILES) $(LUADIR)/src/liblua.a $(LIBS) $(GSL_LIBS)

libluagsl.a: $(LUAGSL_OBJ_FILES)
	$(AR) $@ $?
	$(RANLIB) $@

%.o: %.c
	@echo $(CC) -c $(CFLAGS) $(DEFS) $(INCLUDES) $(LUA_CFLAGS) $<
	@$(CC) -Wp,-MMD,.deps/$(*F).pp -c \
		$(CFLAGS) $(DEFS) $(INCLUDES) \
		$(LUA_CFLAGS) $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

lua-gsl.dll: $(LUAGSL_OBJ_FILES)
	$(CC) -O -shared -o $@ $(LUAGSL_OBJ_FILES) $(LIBS) $(GSL_LIBS) \
		$(LUA_LIBS)
else
# link objects to make static and dynamic libraries.  The .so will be
# left in "./.libs/".  Note that the Lua library and its dependencies are
# not called out on the link line since they are assumed to be part of
# whatever our library is linked to.  We want to avoid duplicate library
# copies, which is a waste of space and can cause run-time problems.
%.lo %.o: %.c
	@echo $(CC) -c $(CFLAGS) $(DEFS) $(INCLUDES) $(LUA_CFLAGS) $<
	@$(LIBTOOL) --mode=compile $(CC) -Wp,-MMD,.deps/$(*F).pp -c \
		$(CFLAGS) $(DEFS) $(INCLUDES) \
		$(LUA_CFLAGS) $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

gsl-shell.o: gsl-shell.c
	$(CC) -c --std=c99 $(CFLAGS) $(DEFS) -I$(LUADIR)/src $(INCLUDES) $(LUA_CFLAGS) $<

gsl-shell: $(LUAGSL_OBJ_FILES) gsl-shell.o
	$(CC) -o $@ gsl-shell.o $(LUAGSL_OBJ_FILES) $(LUADIR)/src/liblua.a $(LIBS) $(GSL_LIBS) -Wl,-E -ldl -lreadline -lhistory -lncurses

liblua-gsl.la lua-gsl.so: $(LUAGSL_LOBJ_FILES) $(LUAGSL_OBJ_FILES)
	$(LIBTOOL) --mode=link $(CC) \
		-rpath $(RPATH) -o liblua-gsl.la $(LUAGSL_LOBJ_FILES) \
		$(GSL_LIBS)
	ln -sf ./.libs/liblua-gsl.so lua-gsl.so
endif

.PHONY: clean all $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	$(RM) *.o *.lo *.la *.so $(TARGETS)
	$(RM) -r ./.libs/

-include $(DEP_FILES)

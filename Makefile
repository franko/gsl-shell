
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
  LUA_DLL = lua-gsl.dll
else
  INCLUDES += -DLUA_USE_LINUX
  CFLAGS += -fpic
  LUA_DLL = lua-gsl.so
endif

ifeq ($(target),mingw)
  TARGETS = gsl-shell.exe
else
  TARGETS = gsl-shell
endif

LUADIR = lua-5.1.4
AR= ar rcu
RANLIB= ranlib

ifeq ($(target),mingw)
  LUA_CFLAGS =
  LUA_LIBS = -L$(LUADIR)/src -llua51
else
  LUA_CFLAGS = -I$(LUADIR)/src
#  LUA_LIBTOOL := $(shell pkg-config lua5.1 --variable=libtool_lib)
#  RPATH=$(shell pkg-config lua5.1 --define-variable=prefix=/usr/local \
 #	--variable=INSTALL_CMOD)
endif

COMPILE = $(CC) --std=c99 $(CFLAGS) $(LUA_CFLAGS) $(DEFS) $(INCLUDES)

LUAGSL_SRC_FILES = math-types.c matrix.c cmatrix.c nlinfit_helper.c \
		fdfsolver.c nlinfit.c cnlinfit.c lua-utils.c linalg.c \
		integ.c ode_solver.c ode.c code.c fft.c lua-gsl.c

SUBDIRS = 

LUAGSL_OBJ_FILES := $(LUAGSL_SRC_FILES:%.c=%.o)

DEP_FILES := $(LUAGSL_SRC_FILES:%.c=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)
LIBS_MAGIC := $(shell mkdir .libs > /dev/null 2>&1 || :)

GSL_LIBS = -lgsl -lgslcblas -lm

all: $(TARGETS)

ifeq ($(target),mingw)

gsl-shell.exe: $(LUAGSL_OBJ_FILES) gsl-shell.o
	$(CC) -o $@ $(LUAGSL_OBJ_FILES) gsl-shell.o $(LUADIR)/src/liblua.a $(LIBS) $(GSL_LIBS)

libluagsl.a: $(LUAGSL_OBJ_FILES)
	$(AR) $@ $?
	$(RANLIB) $@

lua-gsl.dll: $(LUAGSL_OBJ_FILES)
	$(CC) -O -shared -o $@ $(LUAGSL_OBJ_FILES) $(LIBS) $(GSL_LIBS) \
		$(LUA_LIBS)
else

gsl-shell: $(LUAGSL_OBJ_FILES) gsl-shell.o
	$(CC) -o $@ $(LUAGSL_OBJ_FILES) gsl-shell.o $(LUADIR)/src/liblua.a $(LIBS) $(GSL_LIBS) -Wl,-E -ldl -lreadline -lhistory -lncurses

lua-gsl.so: $(LUAGSL_OBJ_FILES)
	$(CC) -shared -o .libs/liblua-gsl.so $(LUAGSL_OBJ_FILES) $(GSL_LIBS)
	ln -sf ./.libs/liblua-gsl.so lua-gsl.so
endif

%.o: %.c
	@echo $(COMPILE) -c $<
	@$(COMPILE) -Wp,-MMD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> .deps/$(*F).P; \
	rm .deps/$(*F).pp


shared: $(LUA_DLL)

.PHONY: clean shared all $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	$(RM) *.o *.lo *.la *.so $(TARGETS)
	$(RM) -r ./.libs/

-include $(DEP_FILES)


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

LUADIR = lua-5.1.4
AR= ar rcu
RANLIB= ranlib

ifeq ($(target),mingw)
# Option for Windows Platform
  DEFS += -DWIN32
  INCLUDES += -I/usr/include
  LIBS += -L/usr/lib
  LUA_CFLAGS = -I$(LUADIR)/src
  LUA_LIBS = -L$(LUADIR)/src -llua51
  LUA_DLL = gsl.dll
  GSL_SHELL = gsl-shell.exe
else
  INCLUDES += -DLUA_USE_LINUX
  LUA_CFLAGS = -I$(LUADIR)/src
  LUA_DLL = gsl.so
  GSL_SHELL = gsl-shell
endif

ifeq ($(enable_shared),yes)
  CFLAGS += -fpic
  DEFS += -DUSE_SEPARATE_NAMESPACE
  TARGETS = $(LUA_DLL)
else
  TARGETS = $(GSL_SHELL)
endif

LUAGSL_SRC_FILES = math-types.c matrix.c nlinfit_helper.c \
		fdfsolver.c nlinfit.c lua-utils.c linalg.c \
		integ.c ode_solver.c ode.c lua-gsl.c

ifeq ($(enable_complex),yes)
  LUAGSL_SRC_FILES += cmatrix.c cnlinfit.c code.c fft.c
  DEFS += -DGSH_HAVE_COMPLEX
endif

COMPILE = $(CC) --std=c99 $(CFLAGS) $(LUA_CFLAGS) $(DEFS) $(INCLUDES)

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

luagsl.a: $(LUAGSL_OBJ_FILES)
	$(AR) $@ $?
	$(RANLIB) $@

gsl.dll: $(LUAGSL_OBJ_FILES)
	$(CC) -O -shared -o $@ $(LUAGSL_OBJ_FILES) $(LIBS) $(GSL_LIBS) \
		$(LUA_LIBS)
else

gsl-shell: $(LUAGSL_OBJ_FILES) gsl-shell.o
	$(CC) -o $@ $(LUAGSL_OBJ_FILES) gsl-shell.o $(LUADIR)/src/liblua.a $(LIBS) $(GSL_LIBS) -Wl,-E -ldl -lreadline -lhistory -lncurses

gsl.so: $(LUAGSL_OBJ_FILES)
	$(CC) -shared -o .libs/gsl.so $(LUAGSL_OBJ_FILES) $(GSL_LIBS)
	ln -sf ./.libs/gsl.so $@
endif

%.o: %.c
	@echo $(COMPILE) -c $<
	@$(COMPILE) -Wp,-MMD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

.PHONY: clean all $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	$(RM) *.o *.lo *.la *.so $(TARGETS)
	$(RM) -r ./.libs/

-include $(DEP_FILES)

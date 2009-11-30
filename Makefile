
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

LUADIR = lua
AR= ar rcu
RANLIB= ranlib

ifeq ($(PLATFORM), none)
nono:
	@echo "You haven't edited 'makeconfig' yet. Set your settings there, then run 'make' again"
endif


ifeq ($(strip $(PLATFORM)), mingw)
# Option for Windows Platform
  DEFS += -DWIN32
  INCLUDES += -I. -I/usr/include -I/usr/pthreads-w32/include
  LIBS += -L/usr/lib -L/usr/pthreads-w32/lib

  LUA_CFLAGS = -I$(LUADIR)/src
  LUA_LIBS = -L$(LUADIR)/src -llua51
  LUA_DLL = gsl.dll

  AGG_HOME = /c/fra/src/agg-2.5
  AGG_CFLAGS = -I$(AGG_HOME)/include
  AGG_LIBS = -L$(AGG_HOME)/src  -L$(AGG_HOME)/src/platform/win32 \
		-lagg -laggplatformwin32
  AGG_TRANS_AFFINE = $(AGG_HOME)/src/agg_trans_affine.o

  GSL_SHELL = gsl-shell.exe
else
  INCLUDES += -I. -DLUA_USE_LINUX

  AGG_LIBS = -lagg -laggplatformX11 -lX11

  LUA_CFLAGS = -I$(LUADIR)/src
  LUA_DLL = gsl.so

  GSL_SHELL = gsl-shell
endif

INCLUDES += -Iagg-plot

ifeq ($(strip $(BUILD_LUA_DLL)), yes)
  CFLAGS += -fpic
  DEFS += -DUSE_SEPARATE_NAMESPACE
  TARGETS = $(LUA_DLL)
else
  SUBDIRS_DEFS += -DGSL_SHELL
  TARGETS = $(GSL_SHELL)
endif

LUAGSL_SRC_FILES = common.c math-types.c matrix.c nlinfit_helper.c \
		fdfsolver.c nlinfit.c lua-utils.c linalg.c \
		integ.c ode_solver.c ode.c random.c randist.c \
		pdf.c cdf.c lua-gsl.c lua-cplot.c gsl-shell.c

AGGMAIN_SRC = agg_main.cpp
AGGMAIN_OBJ = agg_main.o

ifeq ($(strip $(ENABLE_COMPLEX)), yes)
  LUAGSL_SRC_FILES += cmatrix.c cnlinfit.c code.c fft.c
  DEFS += -DLNUM_COMPLEX
  SUBDIRS_DEFS += -DLNUM_COMPLEX
endif

COMPILE = $(CC) --std=c99 $(CFLAGS) $(LUA_CFLAGS) $(DEFS) $(INCLUDES)
CXXCOMPILE = $(CXX) -c

SUBDIRS = lua agg-plot

LUAGSL_OBJ_FILES := $(LUAGSL_SRC_FILES:%.c=%.o)

DEP_FILES := $(LUAGSL_SRC_FILES:%.c=.deps/%.P) $(AGGMAIN_SRC:%.cpp=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)
LIBS_MAGIC := $(shell mkdir .libs > /dev/null 2>&1 || :)

GSL_LIBS = -lgsl -lgslcblas -lm

all: $(SUBDIRS) $(TARGETS)

ifeq ($(PLATFORM), mingw)

gsl-shell.exe: $(LUAGSL_OBJ_FILES) $(AGGMAIN_OBJ)
	$(CC) -Wl,--enable-auto-import -o $@ $(LUAGSL_OBJ_FILES) $(AGGMAIN_OBJ) agg-plot/libaggplot.a $(LUADIR)/src/liblua.a -lpthreadGC2 $(LIBS) $(GSL_LIBS) $(AGG_LIBS) -lgdi32 -lsupc++

luagsl.a: $(LUAGSL_OBJ_FILES)
	$(AR) $@ $?
	$(RANLIB) $@

gsl.dll: $(LUAGSL_OBJ_FILES)
	$(CC) -O -shared -o $@ $(LUAGSL_OBJ_FILES) $(LIBS) $(GSL_LIBS) \
		$(LUA_LIBS)
else

gsl-shell: $(LUAGSL_OBJ_FILES) $(AGGMAIN_OBJ) agg-plot/libaggplot.a
	$(CC) -o $@ $(LUAGSL_OBJ_FILES) $(AGGMAIN_OBJ) $(LUADIR)/src/liblua.a agg-plot/libaggplot.a $(LIBS) $(GSL_LIBS) -Wl,-E -Wl,--allow-multiple-definition -ldl -lreadline -lhistory -lncurses $(AGG_LIBS) -lpthread -lsupc++

gsl.so: $(LUAGSL_OBJ_FILES)
	$(CC) -shared -o .libs/libluagsl.so $(LUAGSL_OBJ_FILES) $(GSL_LIBS)
	ln -sf ./.libs/libluagsl.so $@
endif

%.o: %.c
	@echo $(COMPILE) -c $<
	@$(COMPILE) -Wp,-MMD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

%.o: %.cpp
	@echo $(CXXCOMPILE) -c $< 
	@$(CXXCOMPILE) -Wp,-MMD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

.PHONY: clean all $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) DEFS='"$(SUBDIRS_DEFS)"' PLATFORM=$(strip $(PLATFORM)) -C $@

clean:
	$(MAKE) -C lua clean
	$(RM) *.o *.lo *.la *.so *.dll $(TARGETS)
	$(RM) -r ./.libs/

-include $(DEP_FILES)

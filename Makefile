
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
include makeflags
include make-packages

LUADIR = lua
AR= ar rcu
RANLIB= ranlib

SUBDIRS_DEFS = -DGSL_SHELL

ifeq ($(strip $(PLATFORM)), none)
nono:
	@echo "You haven't edited 'makeconfig' yet. Set your settings there, then run 'make' again"
endif


ifeq ($(strip $(PLATFORM)), mingw)
# Option for Windows Platform
  DEFS += -DWIN32
  INCLUDES += -I. -I/usr/include
  LIBS += -L/usr/lib

  LUA_CFLAGS = -I$(LUADIR)/src
  LUA_LIBS = -L$(LUADIR)/src -llua51
  LUA_DLL = gsl.dll

  GSL_SHELL = gsl-shell.exe
else
  INCLUDES += -I. -DLUA_USE_LINUX
  AGG_LIBS = -lagg -lX11 -lpthread -lsupc++
  LUA_CFLAGS = -I$(LUADIR)/src
  LUA_DLL = gsl.so
  GSL_SHELL = gsl-shell
  PTHREADS_LIBS = -lpthread
endif

SUBDIRS = lua disp

LUAGSL_LIBS = $(LUADIR)/src/liblua.a 

C_SRC_FILES = common.c math-types.c matrix.c nlinfit_helper.c \
		nlinfit.c lua-utils.c linalg.c \
		integ.c ode_solver.c ode.c random.c randist.c \
		pdf.c cdf.c sf.c fmultimin.c gradcheck.c fdfmultimin.c multimin.c \
		lua-gsl.c

ifeq ($(strip $(BUILD_LUA_DLL)), yes)
  CFLAGS += -fpic
  DEFS += -DLUA_MODULE -DUSE_SEPARATE_NAMESPACE
  TARGETS = $(LUA_DLL)
else
  C_SRC_FILES += gsl-shell.c
  SUBDIRS_DEFS += -DGSL_SHELL_LUA
  TARGETS = $(GSL_SHELL)
endif

ifeq ($(strip $(ENABLE_AGG_PLOT)), yes)
  C_SRC_FILES += lua-plot.c
  INCLUDES += $(PTHREADS_CFLAGS) -Iagg-plot
  SUBDIRS += agg-plot
  DEFS += -DAGG_PLOT_ENABLED
  LUAGSL_LIBS += agg-plot/libaggplot.a
  LIBS += $(PTHREADS_LIBS) $(AGG_LIBS)
endif

ifeq ($(strip $(ENABLE_COMPLEX)), yes)
  C_SRC_FILES += cmatrix.c cnlinfit.c code.c fft.c
  DEFS += -DLNUM_COMPLEX
  SUBDIRS_DEFS += -DLNUM_COMPLEX
endif

COMPILE = $(CC) --std=c99 $(CFLAGS) $(LUA_CFLAGS) $(DEFS) $(INCLUDES)
CXXCOMPILE = $(CXX) $(CXXFLAGS) -c

LUAGSL_OBJ_FILES = $(C_SRC_FILES:%.c=%.o) $(CXX_SRC_FILES:%.cpp=%.o)

DEP_FILES := $(C_SRC_FILES:%.c=.deps/%.P) $(CXX_SRC_FILES:%.cpp=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)
LIBS_MAGIC := $(shell mkdir .libs > /dev/null 2>&1 || :)

GSL_LIBS = -lgsl -lgslcblas -lm
LIBS += $(GSL_LIBS)

all: $(SUBDIRS) $(TARGETS)

ifeq ($(PLATFORM), mingw)

gsl-shell.exe: $(LUAGSL_OBJ_FILES) $(LUAGSL_LIBS)
	$(CC) -Wl,--enable-auto-import -o $@ $(LUAGSL_OBJ_FILES) $(LUAGSL_LIBS) $(LIBS)

luagsl.a: $(LUAGSL_OBJ_FILES)
	$(AR) $@ $?
	$(RANLIB) $@

gsl.dll: $(LUAGSL_OBJ_FILES) $(LUAGSL_LIBS)
	$(CC) -O -shared -Wl,--enable-auto-import -o $@ $(LUAGSL_OBJ_FILES) $(LUAGSL_LIBS) $(LIBS)
else

gsl-shell: $(LUAGSL_OBJ_FILES) $(LUAGSL_LIBS)
	$(CC) -o $@ $(LUAGSL_OBJ_FILES) $(LUAGSL_LIBS) $(LIBS) -Wl,-E -ldl -lreadline -lhistory -lncurses

gsl.so: $(LUAGSL_OBJ_FILES) $(LUAGSL_LIBS)
	$(CC) -shared -o .libs/libluagsl.so $(LUAGSL_OBJ_FILES) $(LUAGSL_LIBS) $(LIBS)
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
	$(MAKE) -C agg-plot clean
	$(MAKE) -C lua clean
	$(RM) *.o $(TARGETS)
	$(RM) -r ./.libs/

-include $(DEP_FILES)

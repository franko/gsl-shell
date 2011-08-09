
# Makefile
# 
# Copyright (C) 2009, 2010 Francesco Abbate
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
include makepackages
include makedefs

LUADIR = luajit2

ifeq ($(strip $(PLATFORM)), none)
nono:
	@echo "You haven't edited 'makeconfig' yet. Set your settings there, then run 'make' again"
endif

INCLUDES += -I. $(GSL_INCLUDES)
GSL_SHELL = gsl-shell$(EXE_EXT)
LUA_CFLAGS = -I$(LUADIR)/src

ifeq ($(strip $(PLATFORM)), mingw)
# Option for Windows Platform
  INCLUDES += -I/usr/include
  LDFLAGS += -Wl,--enable-auto-import
  LIBS += -L/usr/lib
else
  LDFLAGS += -Wl,-E
  LIBS += -ldl -lreadline -lhistory -lncurses
endif

SUBDIRS = $(LUADIR)

C_SRC_FILES = gs-types.c lua-utils.c random.c randist.c \
		pdf.c cdf.c sf.c lua-graph.c lua-gsl.c

LUA_BASE_DIRS = 
LUA_BASE_FILES = igsl.lua base.lua integ.lua csv.lua

LUAGSL_LIBS = $(LUADIR)/src/libluajit.a
C_SRC_FILES += gsl-shell-jit.c

DEFS += -DGSL_SHELL_LUA -DLUA_ROOT=$(PREFIX)
TARGETS = $(GSL_SHELL)

ifeq ($(strip $(ENABLE_AGG_PLOT)), yes)
  LUA_BASE_DIRS += pre3d
  LUA_BASE_FILES += draw.lua contour.lua plot3d.lua \
		pre3d/pre3d.lua pre3d/pre3d_shape_utils.lua
  INCLUDES += $(PTHREADS_CFLAGS) -Iagg-plot
  SUBDIRS += agg-plot
  LUAGSL_LIBS += agg-plot/libaggplot.a
  LIBS += $(PTHREADS_LIBS) $(AGG_LIBS) -lsupc++
endif

COMPILE = $(CC) $(CFLAGS) $(LUA_CFLAGS) $(DEFS) $(INCLUDES)
CXXCOMPILE = $(CXX) $(CXXFLAGS) -c
LINK_EXE = $(CC) $(LDFLAGS)

LUAGSL_OBJ_FILES = $(C_SRC_FILES:%.c=%.o) $(CXX_SRC_FILES:%.cpp=%.o)

DEP_FILES := $(C_SRC_FILES:%.c=.deps/%.P) $(CXX_SRC_FILES:%.cpp=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)
LIBS_MAGIC := $(shell mkdir .libs > /dev/null 2>&1 || :)

LIBS += $(GSL_LIBS)

all: $(SUBDIRS) $(TARGETS)

$(GSL_SHELL): $(LUAGSL_OBJ_FILES) $(LUAGSL_LIBS)
	@echo Linking $@
	@$(LINK_EXE) -o $@ $(LUAGSL_OBJ_FILES) $(LUAGSL_LIBS) $(LIBS)

install: $(GSL_SHELL)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp gsl-shell $(DESTDIR)$(PREFIX)/bin
	strip $(DESTDIR)$(PREFIX)/bin/gsl-shell
	mkdir -p $(DESTDIR)$(PREFIX)/lib/gsl-shell
	mkdir -p $(LUA_BASE_DIRS) 
	cp --parents $(LUA_BASE_FILES) $(DESTDIR)$(PREFIX)/lib/gsl-shell
	mkdir -p $(DESTDIR)$(PREFIX)/lib/gsl-shell/examples
	cp examples/*.lua $(DESTDIR)$(PREFIX)/lib/gsl-shell/examples

.PHONY: clean all $(SUBDIRS)

include makerules

$(SUBDIRS):
	cd $@; $(MAKE)

clean:
	$(MAKE) -C agg-plot clean
	$(MAKE) -C $(LUADIR) clean
	$(RM) *.o $(TARGETS)
	$(RM) -r ./.libs/

-include $(DEP_FILES)

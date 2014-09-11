
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

ifneq (,$(findstring Windows,$(OS)))
  HOST_SYS= Windows
else
  HOST_SYS:= $(shell uname -s)
  ifneq (,$(findstring CYGWIN,$(TARGET_SYS)))
    HOST_SYS= Windows
  endif
endif

ifeq ($(strip $(DEBUG)), yes)
  CXXFLAGS = -g -Wall
  DEFS += -LIBGRAPH_DEBUG
else
  CXXFLAGS = -Os -fno-exceptions -ffast-math -fno-rtti -Wall
endif

PTHREAD_DEFS += -D_REENTRANT
CPP_SUP_LIBS = -lsupc++

ifeq ($(strip $(DISABLE_GAMMA_CORR)), yes)
  DEFS += -DDISABLE_GAMMA_CORR
endif

ifeq ($(strip $(DISABLE_SUBPIXEL_LCD)), yes)
  DEFS += -DDISABLE_SUBPIXEL_AA
endif

ifeq ($(HOST_SYS),Windows)
  USER_LIBS_HOME = C:/fra/local
  USER_INCLUDE = $(USER_LIBS_HOME)/include

  INCLUDES += -I$(USER_INCLUDE) -I/usr/include -I/usr/pthreads-w32/include
  LIBS += -L$(USER_LIBS_HOME)/lib

  AGG_INCLUDES = -I$(USER_INCLUDE)/agg2
  AGG_LIBS = -lagg -lgdi32

  FREETYPE_INCLUDES = -I$(USER_INCLUDE)/freetype
  FREETYPE_LIBS = -lfreetype

  PTHREADS_LIBS = -lpthread

  LIBGRAPH_SO = graph.dll

  LDFLAGS += -Wl,--enable-auto-import
  LIBS += -L/usr/lib

  DEFS += -DWIN32
else
  ifeq ($(HOST_SYS),Darwin)

# GWH: determined all of the following flags (except for pthread)
# using the pkg-config utility, except I had to add -lX11 to AGG_LIBS.

  AGG_INCLUDES = $(shell pkg-config libagg --cflags)
  AGG_LIBS = $(shell pkg-config libagg --libs) -lX11

# GWH: pkg-config will include "-Wl,-rpath,/opt/local/lib" in AGG_LIBS.
# If you don't include that, the code won't run unless you first do:
#   export DYLD_FALLBACK_LIBRARY_PATH=/opt/local/lib

  FREETYPE_INCLUDES = $(shell pkg-config freetype2 --cflags)
  FREETYPE_LIBS = $(shell pkg-config freetype2 --libs)

  PTHREADS_LIBS = -lpthread

  LINK_EXE = $(CXX) $(LDFLAGS)
  # Use rsync because the --parents option to cp doesn't exist in
  # Mac OS X
  CP_REL = rsync -R
  LDFLAGS += -L/usr/X11/lib
  else
  AGG_INCLUDES = -I/usr/include/agg2
  AGG_LIBS = -lagg -lX11

  FREETYPE_INCLUDES = -I/usr/include/freetype2
  FREETYPE_LIBS = -lfreetype

  PTHREAD_DEFS += -pthread
  PTHREADS_LIBS = -lpthread
  LDFLAGS += -Wl,-E
  endif
endif

ifeq ($(HOST_SYS),Windows)
  PLATSUP_SRC_FILES = support_win32.cpp agg_platform_support_win32.cpp agg_win32_bmp.cpp
else
  ifeq ($(HOST_SYS),Darwin)
    DEFS += -DDARWIN_MACOSX
  endif
  PLATSUP_SRC_FILES = support_x11.cpp agg_platform_support_x11.cpp
endif

# AR= ar rcu
# RANLIB= ranlib

HOST_CP = cp
HOST_RM = rm -f
CP_REL = cp --parents

CC = gcc
CXX = g++

INCLUDES += $(FREETYPE_INCLUDES) $(AGG_INCLUDES)
LIBS += $(PTHREAD_LIBS)
DEFS += $(PTHREAD_DEFS)

COMPILE = $(CC) $(CFLAGS) $(DEFS) $(INCLUDES)
CXXCOMPILE = $(CXX) $(CXXFLAGS) $(DEFS) $(INCLUDES)

AGGPLOT_CSRC_FILES = gs-types.c lua-properties.c lua-utils.c str.c
AGGPLOT_SRC_FILES = $(PLATSUP_SRC_FILES) printf_check.cpp fonts.cpp gamma.cpp agg_font_freetype.cpp plot.cpp plot-auto.cpp utils.cpp units.cpp colors.cpp markers.cpp draw_svg.cpp canvas_svg.cpp lua-draw.cpp lua-text.cpp text.cpp agg-parse-trans.cpp window_registry.cpp window.cpp lua-plot.cpp canvas-window.cpp bitmap-plot.cpp lua-graph.cpp
AGGPLOT_OBJ_FILES := $(AGGPLOT_SRC_FILES:%.cpp=%.o) $(AGGPLOT_CSRC_FILES:%.c=%.o)
DEP_FILES := $(AGGPLOT_SRC_FILES:%.cpp=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

TARGETS = $(LIBGRAPH_SO)

all: $(TARGETS)

$(LIBGRAPH_SO): $(AGGPLOT_OBJ_FILES)
	$(CC) -shared $(AGGPLOT_OBJ_FILES) -o $@ $(LIBS) $(AGG_LIBS) $(FREETYPE_LIBS) -L../luajit2/src -llua51 -lpthread -lsupc++
	strip --strip-unneeded $@

# libaggplot.a: $(AGGPLOT_OBJ_FILES)
# 	@echo Archive $@
# 	@$(AR) $@ $?
# 	@$(RANLIB) $@

%.o: %.cpp
	@echo Compiling $<
	@$(CXXCOMPILE) -Wp,-MMD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

%.o: %.c
	@echo Compiling $<
	@$(COMPILE) -Wp,-MMD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

.PHONY: clean all

clean:
	$(HOST_RM) *.o *.lo *.la *.so *.dll $(TARGETS)

-include $(DEP_FILES)

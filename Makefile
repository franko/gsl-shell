
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
  DISPLAY_SUFFIX = win32

  USER_LIBS_HOME = C:/fra/local
  USER_INCLUDE = $(USER_LIBS_HOME)/include

  INCLUDES += -I$(USER_INCLUDE) -I/usr/include -I/usr/pthreads-w32/include
  LIBS += -L$(USER_LIBS_HOME)/lib

  AGG_INCLUDES = -I$(USER_INCLUDE)/agg2
  AGG_LIBS = -lagg -lgdi32

  FREETYPE_INCLUDES = -I$(USER_INCLUDE)/freetype
  FREETYPE_LIBS = -lfreetype

  PTHREADS_LIBS = -lpthread

  LIBGRAPH_SO = graphcore.dll
  LIBNATWIN_SO = natwin.dll

  LUA_INCLUDES = -I../luajit2/src
  LUA_LIBS = -L../luajit2/src -llua51

  LDFLAGS += -Wl,--enable-auto-import
  LIBS += -L/usr/lib

  DEFS += -DWIN32
else
  X11_INCLUDES = $(shell pkg-config x11 --cflags)
  X11_LIBS = $(shell pkg-config x11 --libs)

# GWH (for OS X): pkg-config will include "-Wl,-rpath,/opt/local/lib" in AGG_LIBS.
# If you don't include that, the code won't run unless you first do:
#   export DYLD_FALLBACK_LIBRARY_PATH=/opt/local/lib

  AGG_INCLUDES = $(shell pkg-config libagg --cflags)
  AGG_LIBS = $(shell pkg-config libagg --libs)

  FREETYPE_INCLUDES = $(shell pkg-config freetype2 --cflags)
  FREETYPE_LIBS = $(shell pkg-config freetype2 --libs)

  LUA_INCLUDES = $(shell pkg-config --cflags luajit)
  LUA_LIBS = $(shell pkg-config --libs luajit)

  PTHREAD_DEFS += -pthread
  PTHREAD_LIBS = -lpthread

  ifeq ($(HOST_SYS),Darwin)
    LINK_EXE = $(CXX) $(LDFLAGS)
    # Use rsync because the --parents option to cp doesn't exist in
    # Mac OS X
    CP_REL = rsync -R
    LDFLAGS += -Wl,-E
  endif

  DISPLAY_SUFFIX = x11

  LIBGRAPH_SO = libgraphcore.so
  LIBNATWIN_SO = libnatwin.so
endif

ifeq ($(strip $(DEST_DIR)), )
  DEST_PREFIX = $(PREFIX)
else
  DEST_PREFIX = $(DEST_DIR)/$(PREFIX)
endif
LUA_PATH = $(DEST_PREFIX)/share/lua/5.1
LUA_DLLPATH = $(DEST_PREFIX)/lib/lua/5.1
SYSTEM_LIBPATH = $(DEST_PREFIX)/lib

ifeq ($(HOST_SYS),Windows)
  PLATFORM_NATWIN_SRC_FILES = agg_platform_support_win32.cpp agg_win32_bmp.cpp
  PLATFORM_PLOT_SRC_FILES = agg_win32_bmp.cpp
else
  ifeq ($(HOST_SYS),Darwin)
    DEFS += -DDARWIN_MACOSX
  endif
  PLATFORM_NATWIN_SRC_FILES = agg_platform_support_x11.cpp
endif

GRAPH_LUA_SRC = graph/init.lua graph/contour.lua

HOST_CP = cp
HOST_RM = rm -f
CP_REL = cp --parents

CC = gcc
CXX = g++

INCLUDES += $(LUA_INCLUDES) $(FREETYPE_INCLUDES) $(X11_INCLUDES) $(AGG_INCLUDES)
LIBS += $(FREETYPE_LIBS) $(AGG_LIBS) $(X11_LIBS) $(LUA_LIBS) $(PTHREAD_LIBS)
DEFS += $(PTHREAD_DEFS)

COMPILE = $(CC) $(CFLAGS) $(DEFS) $(INCLUDES)
CXXCOMPILE = $(CXX) $(CXXFLAGS) $(DEFS) $(INCLUDES)

NATWIN_CSRC_FILES =
NATWIN_CPPSRC_FILES = $(PLATFORM_NATWIN_SRC_FILES) canvas-window.cpp window.cpp

PLOT_CSRC_FILES = str.c gs-types.c lua-utils.c lua-properties.c
PLOT_CPPSRC_FILES = gamma.cpp printf_check.cpp utils.cpp window_registry.cpp fonts_search_$(DISPLAY_SUFFIX).cpp $(PLATFORM_PLOT_SRC_FILES) image_write_$(DISPLAY_SUFFIX).cpp fonts.cpp agg_font_freetype.cpp plot.cpp plot-auto.cpp units.cpp colors.cpp markers.cpp draw_svg.cpp canvas_svg.cpp lua-draw.cpp lua-text.cpp text.cpp agg-parse-trans.cpp lua-plot.cpp bitmap-plot.cpp lua-graph.cpp
PLOT_OBJ_FILES := $(PLOT_CSRC_FILES:%.c=%.o) $(PLOT_CPPSRC_FILES:%.cpp=%.o)

NATWIN_OBJ_FILES := $(NATWIN_CSRC_FILES:%.c=%.o) $(NATWIN_CPPSRC_FILES:%.cpp=%.o)

DEP_FILES := $(NATWIN_CPPSRC_FILES:%.cpp=.deps/%.P) $(NATWIN_CSRC_FILES:%.c=.deps/%.P) $(PLOT_CPPSRC_FILES:%.cpp=.deps/%.P) $(PLOT_CSRC_FILES:%.c=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

TARGETS = $(LIBGRAPH_SO)

all: $(LIBGRAPH_SO) $(LIBNATWIN_SO)

$(LIBGRAPH_SO): $(PLOT_OBJ_FILES)
	$(CC) -shared $(PLOT_OBJ_FILES) -o $@ $(LIBS) -lsupc++
	strip --strip-unneeded $@

$(LIBNATWIN_SO): $(NATWIN_OBJ_FILES) $(LIBGRAPH_SO)
	$(CC) -shared $(NATWIN_OBJ_FILES) -o $@ $(LIBS) -L. -lgraphcore -lsupc++
	strip --strip-unneeded $@

install: $(TARGETS)
	mkdir -p $(LUA_PATH)
	mkdir -p $(LUA_DLLPATH)
	cp $(LIBGRAPH_SO) $(LUA_DLLPATH)/graphcore.so
	cp $(LIBNATWIN_SO) $(LUA_DLLPATH)/natwin.so
	ln -s $(LUA_DLLPATH)/graphcore.so $(SYSTEM_LIBPATH)/libgraphcore.so
	$(CP_REL) $(GRAPH_LUA_SRC) $(LUA_PATH)

clean:
	$(HOST_RM) *.o *.so *.dll $(TARGETS)

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

-include $(DEP_FILES)

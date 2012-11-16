
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

GSH_BASE_DIR = ..

include $(GSH_BASE_DIR)/makeconfig
include $(GSH_BASE_DIR)/make-system-detect
include $(GSH_BASE_DIR)/makepackages
include $(GSH_BASE_DIR)/makedefs

GSL_SHELL_GUI = gsl-shell-gui$(EXE_EXT)

LUADIR = $(GSH_BASE_DIR)/luajit2

LUA_DLL = $(LUADIR)/src/lua51.dll
GSL_SHELL_LIBS = $(LUA_DLL) $(GSH_BASE_DIR)/agg-plot/libaggplot.a $(GSH_BASE_DIR)/lua-gsl/libluagsl.a

INCLUDES += -I$(GSH_BASE_DIR) -I$(GSH_BASE_DIR)/lua-gsl -I$(GSH_BASE_DIR)/agg-plot -I$(LUADIR)/src -I$(GSH_BASE_DIR)/cpp-utils

INCLUDES += $(AGG_INCLUDES) $(FOX_INCLUDES) $(FREETYPE_INCLUDES) $(PTHREADS_CFLAGS)
LIBS += $(GSL_SHELL_LIBS) $(FOX_LIBS) $(AGG_LIBS) $(FREETYPE_LIBS) $(PTHREADS_LIBS) $(GSL_LIBS) -lsupc++ -lm

FOXGUI_SRC_FILES = io_thread.cpp window_surface.cpp fx_console.cpp redirect.cpp gsl_shell_interp.cpp gsl_shell_thread.cpp fox_gsl_shell.cpp gsl_shell_window.cpp window_part.cpp fx_plot_canvas.cpp fx_plot_window.cpp lua_plot_window.cpp gsl_shell_app.cpp gsl-shell-fox.cpp
FOXGUI_OBJ_FILES := $(FOXGUI_SRC_FILES:%.cpp=%.o)

DEP_FILES := $(FOXGUI_SRC_FILES:%.cpp=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

CXXCOMPILE = $(CC) $(CXXFLAGS) $(DEFS) $(INCLUDES)

ifeq ($(HOST_SYS),Windows)
  LDFLAGS += -mwindows
endif

TARGETS = $(GSL_SHELL_GUI)

all: $(TARGETS)

$(GSL_SHELL_GUI): $(FOXGUI_OBJ_FILES) $(GSL_SHELL_LIBS)
	@echo Linking $@
	@$(LINK_EXE) -o $@ $(FOXGUI_OBJ_FILES) $(LIBS) $(LDFLAGS) $(LUA_DLL)

include $(GSH_BASE_DIR)/makerules

.PHONY: clean all

clean:
	$(HOST_RM) *.o *.lo *.la *.so *.dll $(TARGETS)

-include $(DEP_FILES)

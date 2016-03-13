
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

INCLUDES += -I$(GSH_BASE_DIR) -I$(GSH_BASE_DIR)/lua-gsl -I$(GSH_BASE_DIR)/agg-plot -I$(GSH_BASE_DIR)/cpp-utils -I$(GSH_BASE_DIR)/lang-toolkit/src $(LUAJIT_INCLUDES)
INCLUDES += $(AGG_INCLUDES) $(FOX_INCLUDES) $(FREETYPE_INCLUDES) $(PTHREADS_CFLAGS)
DEFS += $(PTHREAD_DEFS) $(GSL_SHELL_DEFS)

FOXGUI_SRC_FILES = io_thread.cpp window_surface.cpp MyFXTextKeylog.cpp MyFXNotebook.cpp LuaFXConsole.cpp redirect.cpp gsl_shell_interp.cpp gsl_shell_thread.cpp fox_gsl_shell.cpp gsl_shell_window.cpp window_part.cpp fx_plot_canvas.cpp fx_plot_window.cpp lua_plot_window.cpp gsl_shell_app.cpp gsl-shell-fox.cpp
FOXGUI_OBJ_FILES := $(FOXGUI_SRC_FILES:%.cpp=%.o)
DEP_FILES := $(FOXGUI_SRC_FILES:%.cpp=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

TARGETS = libfoxgui.a

all: $(TARGETS)
	@$(HOST_CP) libfoxgui.a $(GSH_LIBDIR)

libfoxgui.a: $(FOXGUI_OBJ_FILES)
	@echo Archive $@
	@$(AR) $@ $?
	@$(RANLIB) $@

include $(GSH_BASE_DIR)/makerules

.PHONY: clean all

clean:
	$(HOST_RM) *.o *.lo *.la *.so *.dll $(TARGETS)

-include $(DEP_FILES)

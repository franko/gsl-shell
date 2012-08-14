
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
include make-system-detect
include makepackages
include makedefs

LUADIR = luajit2

INCLUDES += -I. $(GSL_INCLUDES) -Iagg-plot -Ilua-gsl
GSL_SHELL = gsl-shell$(EXE_EXT)
LUA_CFLAGS = -I$(LUADIR)/src

ifeq ($(HOST_SYS),Windows)
  INCLUDES += -I/usr/include
  LDFLAGS += -Wl,--enable-auto-import
  LIBS += -L/usr/lib -lsupc++
else
  ifeq ($(HOST_SYS),Darwin)
    LDFLAGS += -L/usr/X11/lib -undefined dynamic_lookup -pagezero_size 10000 -image_base 100000000
    LIBS += -ldl -lreadline -lncurses
  else
    LDFLAGS += -Wl,-E
    LIBS += -ldl -lreadline -lhistory -lncurses -lsupc++
  endif
endif

SUBDIRS = $(LUADIR) lua-gsl

C_SRC_FILES =

ifeq ($(strip $(USE_READLINE)),yes)
  C_SRC_FILES += completion.c
endif

LUA_BASE_FILES = bspline.lua fft-init.lua integ-init.lua template.lua check.lua graph-init.lua rng.lua rnd.lua randist.lua iter.lua time.lua gsl-check.lua linfit.lua roots.lua contour.lua gsl.lua matrix.lua csv.lua gslext.lua num.lua demo-init.lua import.lua plot3d.lua sf.lua vegas.lua eigen.lua

DEMOS_LIST = bspline fft plot wave-particle fractals ode nlinfit integ anim linfit contour svg graphics sf vegas
LUA_TEMPLATES = gauss-kronrod-x-wgs qag rk8pd lmfit qng rkf45 ode-defs rk4 sf-defs vegas-defs

LUA_BASE_FILES += $(DEMOS_LIST:%=demos/%.lua)
LUA_BASE_FILES += $(LUA_TEMPLATES:%=templates/%.lua.in)

LUAGSL_LIBS += $(LUADIR)/src/libluajit.a

C_SRC_FILES += gsl-shell-jit.c

TARGETS = $(GSL_SHELL)

# files and flags related to the pre3d modules
LUA_BASE_FILES += pre3d/pre3d.lua pre3d/pre3d_shape_utils.lua
INCLUDES += $(PTHREADS_CFLAGS) -Iagg-plot
SUBDIRS += agg-plot fox-gui
LUAGSL_LIBS += agg-plot/libaggplot.a
LIBS += $(AGG_LIBS) $(FREETYPE_LIBS) $(PTHREADS_LIBS)

LUAGSL_LIBS += lua-gsl/libluagsl.a

COMPILE = $(CC) $(CFLAGS) $(LUA_CFLAGS) $(DEFS) $(INCLUDES)

LUAGSL_OBJ_FILES = $(C_SRC_FILES:%.c=%.o)

DEP_FILES := $(C_SRC_FILES:%.c=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)
LIBS_MAGIC := $(shell mkdir .libs > /dev/null 2>&1 || :)

INSTALL_BIN_DIR = $(DESTDIR)$(PREFIX)/bin
INSTALL_LIB_DIR = $(DESTDIR)$(PREFIX)/share/$(PACKAGE_NAME)/$(PACKAGE_VERSION)

LIBS += $(GSL_LIBS)

all: $(SUBDIRS) $(TARGETS)

$(GSL_SHELL): $(LUAGSL_OBJ_FILES) $(LUAGSL_LIBS)
	@echo Linking $@
	@$(LINK_EXE) -o $@ $(LUAGSL_OBJ_FILES) $(LUAGSL_LIBS) $(LIBS)

install: $(GSL_SHELL)
	mkdir -p $(INSTALL_BIN_DIR)
	cp $(GSL_SHELL) $(INSTALL_BIN_DIR)
	strip $(INSTALL_BIN_DIR)/$(GSL_SHELL)
	mkdir -p $(INSTALL_LIB_DIR)
	cp --parents $(LUA_BASE_FILES) $(INSTALL_LIB_DIR)

.PHONY: clean all $(SUBDIRS)

include makerules

$(SUBDIRS):
	cd $@; $(MAKE)

clean:
	$(MAKE) -C agg-plot clean
	$(MAKE) -C $(LUADIR) clean
	$(HOST_RM) *.o $(TARGETS)
	$(HOST_RM) -r ./.libs/

-include $(DEP_FILES)

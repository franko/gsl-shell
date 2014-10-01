
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

GSH_BASE_DIR = .

INCLUDES += -I. $(GSL_INCLUDES) -Ilua-gsl
LIBS += $(PTHREAD_LIBS)
DEFS += $(PTHREAD_DEFS) $(GSL_SHELL_DEFS)
CFLAGS += $(LUA_CFLAGS)

ifeq ($(HOST_SYS),Windows)
  INCLUDES += -I/usr/include
  LIBS += -L/usr/lib -lsupc++
    LIBGRAPH_SO = graphcore.dll
else
  ifeq ($(HOST_SYS),Darwin)
    LDFLAGS += -L/usr/X11/lib -undefined dynamic_lookup -pagezero_size 10000 -image_base 100000000
    LIBS += -ldl -lreadline -lncurses
  else
    LIBS += -ldl -lreadline -lhistory -lncurses -lsupc++
  endif
  TARGET_LINK_DEP = libluajit-$(ABIVER).so.$(MAJVER)
  LIBGRAPH_SO = libgraphcore.so
endif

FOXGUI_LDFLAGS = -lsupc++ -lm
ifeq ($(HOST_SYS),Windows)
  FOXGUI_LDFLAGS += -mwindows
else
  FOXGUI_LDFLAGS += -ldl
endif

C_SRC_FILES =

ifeq ($(strip $(USE_READLINE)),yes)
  INCLUDES += $(RL_INCLUDES)
  LIBS += $(RL_LIBS)
  C_SRC_FILES += completion.c
endif

LUA_BASE_FILES = bspline.lua fft-init.lua integ-init.lua template.lua check.lua \
	graph-init.lua rng.lua rnd.lua randist.lua iter.lua time.lua gsl-check.lua linfit.lua \
	roots.lua contour.lua gsl.lua matrix.lua csv.lua gslext.lua num.lua demo-init.lua \
	import.lua plot3d.lua sf.lua vegas.lua eigen.lua help.lua cgdt.lua expr-actions.lua \
	expr-lexer.lua expr-parse.lua expr-print.lua gdt-factors.lua gdt-interp.lua gdt-expr.lua \
	gdt-hist.lua gdt-lm.lua gdt.lua gdt-parse-csv.lua gdt-plot.lua lm-expr.lua \
	lm-helpers.lua algorithm.lua monomial.lua linfit_rank.lua matrix-power.lua

HELP_FILES = graphics matrix iter integ ode nlfit vegas rng fft
DEMOS_LIST = bspline fft plot wave-particle fractals ode nlinfit integ anim linfit contour svg graphics sf vegas gdt-lm
LUA_TEMPLATES = gauss-kronrod-x-wgs qag rk8pd lmfit qng rkf45 ode-defs rk4 sf-defs vegas-defs rnd-defs
EXAMPLES_FILES_SRC = am-women-weight perf-julia metro-lm-example exam

LUA_BASE_FILES += $(DEMOS_LIST:%=demos/%.lua)
LUA_BASE_FILES += $(LUA_TEMPLATES:%=templates/%.lua.in)
LUA_BASE_FILES += $(HELP_FILES:%=help/%.lua)

EXAMPLES_FILES := $(EXAMPLES_FILES_SRC:%=examples/%.csv)

C_SRC_FILES += gsl-shell.c

TARGETS = $(GSL_SHELL) # $(GSL_SHELL_GUI)

# files and flags related to the pre3d modules
LUA_BASE_FILES += pre3d/pre3d.lua pre3d/pre3d_shape_utils.lua
INCLUDES += $(PTHREADS_CFLAGS)
LUAGSL_LIBS += $(GSH_LIBDIR)/libluagsl.a
LIBS += $(AGG_LIBS) $(FREETYPE_LIBS) $(PTHREADS_LIBS)

ifneq ($(BUILDMODE),dynamic)
  LUAGSL_LIBS += $(GSH_LIBDIR)/libluajit.a
endif
ifeq ($(BUILDMODE),dynamic)
  GSL_SHELL_DEP = $(LUAJIT_SO) $(TARGET_LINK_DEP)
  LIBS += -L$(GSH_DLL_LIBDIR) -l$(LUAJIT_DLL)
endif

LUAGSL_LIBS += $(GSH_LIBDIR)/libluagsl.a $(GSH_LIBDIR)/liblang.a $(GSH_LIBDIR)/libgdt.a

LUAGSL_OBJ_FILES = $(C_SRC_FILES:%.c=%.o)
DEP_FILES := $(C_SRC_FILES:%.c=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)
LIBS_MAGIC := $(shell mkdir .libs > /dev/null 2>&1 || :)

ifeq ($(HOST_SYS),Windows)
  INSTALL_BIN_DIR = $(DESTDIR)$(PREFIX)
  INSTALL_LIB_DIR = $(DESTDIR)$(PREFIX)/$(PACKAGE_NAME)
  INSTALL_SYS_LIB_DIR = $(INSTALL_BIN_DIR)
else
  INSTALL_BIN_DIR = $(DESTDIR)$(PREFIX)/bin
  INSTALL_LIB_DIR = $(DESTDIR)$(PREFIX)/share/$(PACKAGE_NAME)/$(PACKAGE_VERSION)
  INSTALL_SYS_LIB_DIR = $(DESTDIR)$(PREFIX)/lib
endif

LIBS += $(GSL_LIBS) -lm

SUBDIRS := $(LUADIR) lang lua-gsl graph gdt
FOXGUI_DIR := fox-gui

FOXGUI_LIB = $(GSH_LIBDIR)/libfoxgui.a

all: $(TARGETS) $(SUBDIRS) # $(FOXGUI_DIR)

subdirs: $(SUBDIRS) $(FOXGUI_DIR)

$(SUBDIRS):
	$(MAKE) -C $@

$(FOXGUI_DIR):
	$(MAKE) -C $@

$(GSH_LIBDIR)/libluajit.a: $(LUADIR)
$(GSH_LIBDIR)/liblang.a: lang
$(GSH_LIBDIR)/libluagsl.a: lua-gsl
$(GSH_LIBDIR)/libgdt.a: gdt
$(FOXGUI_LIB): $(FOXGUI_DIR)
$(LUAJIT_SO): $(LUADIR)

$(GSL_SHELL): $(LUAGSL_OBJ_FILES) $(LUAGSL_LIBS) $(GSL_SHELL_DEP) $(SUBDIRS)
	@echo Linking $@
	$(LINK_EXE) -o $@ $(LUAGSL_OBJ_FILES) $(LUAGSL_LIBS) $(LIBS)
	cp graph/$(LIBGRAPH_SO) .

$(GSL_SHELL_GUI): $(FOXGUI_LIB) $(LUAGSL_LIBS) $(GSL_SHELL_DEP) $(SUBDIRS) $(FOXGUI_DIR)
	@echo Linking $@
	$(LINK_EXE) -o $@ $(FOXGUI_LIB) $(LUAGSL_LIBS) $(LIBS) $(FOX_LIBS) $(FOXGUI_LDFLAGS) $(CPP_SUP_LIBS)

install: $(GSL_SHELL) $(GSL_SHELL_GUI)
	mkdir -p $(INSTALL_BIN_DIR)
	cp $(GSL_SHELL) $(INSTALL_BIN_DIR)
	cp $(GSL_SHELL_GUI) $(INSTALL_BIN_DIR)
	test -f $(LUAJIT_SO) && \
	  mkdir -p $(INSTALL_SYS_LIB_DIR) && \
	  cp $(LUAJIT_SO) $(INSTALL_SYS_LIB_DIR) && \
	  cd $(INSTALL_SYS_LIB_DIR) && \
	  ln -s libluajit.so libluajit-$(ABIVER).so && \
	  ln -s libluajit-$(ABIVER).so libluajit-$(ABIVER).so.$(MAJVER) || :
	strip $(INSTALL_BIN_DIR)/$(GSL_SHELL)
	strip $(INSTALL_BIN_DIR)/$(GSL_SHELL_GUI)
	mkdir -p $(INSTALL_LIB_DIR)
	$(CP_REL) $(LUA_BASE_FILES) $(INSTALL_LIB_DIR)
	$(CP_REL) $(EXAMPLES_FILES) $(INSTALL_BIN_DIR)

.PHONY: clean all subdirs $(SUBDIRS) $(FOXGUI_DIR)

include makerules

libluajit-$(ABIVER).so.$(MAJVER): $(LUAJIT_SO)
	$(HOST_RM) libluajit-$(ABIVER).so.$(MAJVER) libluajit-$(ABIVER).so
	ln -s libluajit.so libluajit-$(ABIVER).so
	ln -s libluajit-$(ABIVER).so libluajit-$(ABIVER).so.$(MAJVER)

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	$(MAKE) -C $(FOXGUI_DIR) clean
	$(HOST_RM) *.o *.dll *.so
	$(HOST_RM) -r ./.libs/

-include $(DEP_FILES)

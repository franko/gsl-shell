
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

DEBIAN_BUILD_DIR = debian_build
DEB_ARCH := $(shell dpkg-architecture -qDEB_HOST_ARCH)
DEBIAN_PACKAGE := $(PACKAGE_NAME)_$(VERSION)-1_$(DEB_ARCH).deb

GSH_BASE_DIR = .

INCLUDES += -I. $(GSL_INCLUDES) -Iagg-plot -Ilua-gsl
LIBS += $(PTHREAD_LIBS)
DEFS += $(PTHREAD_DEFS) $(GSL_SHELL_DEFS)
GSH_CFLAGS += $(LUA_CFLAGS)

ifeq ($(HOST_SYS),Windows)
  INCLUDES += -I/usr/include
  LIBS += -L/usr/lib
else
  ifeq ($(HOST_SYS),Darwin)
    LDFLAGS += -L/usr/X11/lib -undefined dynamic_lookup -pagezero_size 10000 -image_base 100000000
    LIBS += -ldl -lreadline -lncurses
  else
    LIBS += -ldl -lreadline -lhistory -lncurses
  endif
  TARGET_LINK_DEP = libluajit-$(ABIVER).so.$(MAJVER)
endif

FOXGUI_LDFLAGS = -lm
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

LUA_BASE_FILES = bspline.lua fft.lua fftw3/init.lua fftw3/cdefs.lua fftw3/defines.lua integ-init.lua template.lua check.lua \
	graph-init.lua rng.lua rnd.lua randist.lua iter.lua time.lua gsl-check.lua linfit.lua \
	roots.lua contour.lua gsl.lua matrix.lua csv.lua gslext.lua num.lua demo-init.lua \
	import.lua plot3d.lua sf.lua vegas.lua eigen.lua help.lua cgdt.lua expr-actions.lua \
	expr-lexer.lua expr-parse.lua expr-print.lua gdt-factors.lua gdt-interp.lua gdt-expr.lua \
	gdt-hist.lua gdt-lm.lua gdt.lua gdt-parse-csv.lua gdt-plot.lua lm-expr.lua \
	lm-helpers.lua algorithm.lua monomial.lua linfit_rank.lua matrix-power.lua base.lua

HELP_FILES = graphics matrix iter integ ode nlfit vegas rng fft
DEMOS_LIST = bspline fft plot wave-particle fractals ode nlinfit integ anim linfit contour svg graphics sf vegas gdt-lm
LUA_TEMPLATES = gauss-kronrod-x-wgs qag rk8pd lmfit qng rkf45 ode-defs rk4 sf-defs vegas-defs rnd-defs
EXAMPLES_FILES_SRC = am-women-weight perf-julia metro-lm-example exam

LUA_BASE_FILES += $(DEMOS_LIST:%=demos/%.lua)
LUA_BASE_FILES += $(LUA_TEMPLATES:%=templates/%.lua.in)
LUA_BASE_FILES += $(HELP_FILES:%=help/%.lua)

EXAMPLES_FILES := $(EXAMPLES_FILES_SRC:%=examples/%.csv)

C_SRC_FILES += gsl-shell-jit.c

TARGETS = $(GSL_SHELL) $(GSL_SHELL_GUI)

# files and flags related to the pre3d modules
LUA_BASE_FILES += pre3d/pre3d.lua pre3d/pre3d_shape_utils.lua
INCLUDES += $(PTHREADS_CFLAGS) -Iagg-plot
LUAGSL_LIBS += $(GSH_LIBDIR)/libaggplot.a
LIBS += $(AGG_LIBS) $(FREETYPE_LIBS) $(PTHREADS_LIBS)

ifneq ($(BUILDMODE),dynamic)
  LUAGSL_LIBS += $(GSH_LIBDIR)/libluajit.a
endif
ifeq ($(BUILDMODE),dynamic)
  GSL_SHELL_DEP = $(LUAJIT_SO) $(TARGET_LINK_DEP)
  LIBS += -L$(GSH_DLL_LIBDIR) -l$(LUAJIT_DLL)
endif

LUAGSL_LIBS += $(GSH_LIBDIR)/libluagsl.a $(GSH_LIBDIR)/libgdt.a

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

SUBDIRS := $(LUADIR) lua-gsl agg-plot gdt
FOXGUI_DIR := fox-gui

FOXGUI_LIB = $(GSH_LIBDIR)/libfoxgui.a

all: $(TARGETS) $(SUBDIRS) $(FOXGUI_DIR)

subdirs: $(SUBDIRS) $(FOXGUI_DIR)

$(SUBDIRS):
	$(MAKE) -C $@

$(FOXGUI_DIR):
	$(MAKE) -C $@

$(GSH_LIBDIR)/libluajit.a: $(LUADIR)
$(GSH_LIBDIR)/libluagsl.a: lua-gsl
$(GSH_LIBDIR)/libgdt.a: gdt
$(GSH_LIBDIR)/libaggplot.a: agg-plot
$(FOXGUI_LIB): $(FOXGUI_DIR)
$(LUAJIT_SO): $(LUADIR)

$(GSL_SHELL): $(LUAGSL_OBJ_FILES) $(LUAGSL_LIBS) $(GSL_SHELL_DEP) $(SUBDIRS)
	@echo Linking $@
	$(LINK_EXE) -o $@ $(LUAGSL_OBJ_FILES) $(LUAGSL_LIBS) $(LIBS)

$(GSL_SHELL_GUI): $(FOXGUI_LIB) $(LUAGSL_LIBS) $(GSL_SHELL_DEP) $(SUBDIRS) $(FOXGUI_DIR)
	@echo Linking $@
	$(LINK_EXE) -o $@ $(FOXGUI_LIB) $(LUAGSL_LIBS) $(LIBS) $(FOX_LIBS) $(FOXGUI_LDFLAGS)

define install-to-dir =
mkdir -p $1$(PREFIX)/bin
mkdir -p $1$(PREFIX)/share/applications
mkdir -p $1$(PREFIX)/share/icons/hicolor/128x128/apps
mkdir -p $1$(PREFIX)/share/$(PACKAGE_NAME)/$(PACKAGE_VERSION)
cp $(GSL_SHELL_GUI) $(GSL_SHELL) $1$(PREFIX)/bin
strip $1$(PREFIX)/bin/$(GSL_SHELL) $1$(PREFIX)/bin/$(GSL_SHELL_GUI)
test -f $(LUAJIT_SO) && \
  mkdir -p $1$(PREFIX)/lib && \
  cp $(LUAJIT_SO) $1$(PREFIX)/lib && \
  cd $1$(PREFIX)/lib && \
  ln -s libluajit.so libluajit-$(ABIVER).so && \
  ln -s libluajit-$(ABIVER).so libluajit-$(ABIVER).so.$(MAJVER) || :
cp resources/gsl-shell.desktop $1$(PREFIX)/share/applications
cp resources/gsl-shell-128.png $1$(PREFIX)/share/icons/hicolor/128x128/apps/gsl-shell.png
$(CP_REL) $(LUA_BASE_FILES) $1$(PREFIX)/share/$(PACKAGE_NAME)/$(PACKAGE_VERSION)
$(CP_REL) $(EXAMPLES_FILES) $1$(PREFIX)/bin
endef

install: $(GSL_SHELL) $(GSL_SHELL_GUI)
	$(call install-to-dir,$(DESTDIR))

debian: $(DEBIAN_PACKAGE)

$(DEBIAN_PACKAGE): $(GSL_SHELL) $(GSL_SHELL_GUI)
	$(HOST_RM) -r $(DEBIAN_BUILD_DIR)
	$(call install-to-dir,$(DEBIAN_BUILD_DIR))
	fakeroot bash debian/build.sh $(PACKAGE_NAME) $(VERSION)

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


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
include $(GSH_BASE_DIR)/makeflags
include $(GSH_BASE_DIR)/make-packages

LUADIR = $(GSH_BASE_DIR)/lua
AR= ar rcu
RANLIB= ranlib

ifeq ($(PLATFORM), none)
nono:
	@echo "You haven't edited 'makeconfig' yet. Set your settings there, then run 'make' again"
endif

ifeq ($(strip $(PLATFORM)), mingw)
# Option for Windows Platform
  DEFS += -DWIN32
  INCLUDES += -I/usr/include -I/usr/pthreads-w32/include
  PLATSUP_SRC_FILES = agg_platform_support_win32.cpp agg_win32_bmp.cpp
else
  PLATSUP_SRC_FILES = agg_platform_support_x11.cpp
endif

INCLUDES += $(AGG_INCLUDES) -I$(GSH_BASE_DIR) -I$(LUADIR)/src

AGGPLOT_SRC_FILES = $(PLATSUP_SRC_FILES) utils.cpp units.cpp colors.cpp markers.cpp lua-draw.cpp lua-text.cpp text.cpp drawable.cpp agg-parse-trans.cpp canvas-window.cpp plot-window.cpp

AGGPLOT_OBJ_FILES := $(AGGPLOT_SRC_FILES:%.cpp=%.o)

DEP_FILES := $(AGGPLOT_SRC_FILES:%.cpp=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

CXXCOMPILE = $(CXX) $(CXXFLAGS) $(DEFS) $(INCLUDES) 

TARGETS = libaggplot.a

all: $(TARGETS)

libaggplot.a: $(AGGPLOT_OBJ_FILES)
	$(AR) $@ $?
	$(RANLIB) $@

%.o: %.cpp
	@echo $(CXXCOMPILE) -c $< 
	@$(CXXCOMPILE) -Wp,-MMD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

%.o: %.c
	@echo $(COMPILE) -c $<
	@$(COMPILE) -Wp,-MMD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

.PHONY: clean all

clean:
	$(RM) *.o *.lo *.la *.so *.dll $(TARGETS)

-include $(DEP_FILES)

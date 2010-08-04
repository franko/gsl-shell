
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

GSH_BASE_DIR = ../..

include $(GSH_BASE_DIR)/makeconfig
include $(GSH_BASE_DIR)/makeflags

LUADIR = $(GSH_BASE_DIR)/lua
AR= ar rcu
RANLIB= ranlib

ifeq ($(PLATFORM), none)
nono:
	@echo "You haven't edited 'makeconfig' yet. Set your settings there, then run 'make' again"
endif

LUA_CFLAGS  = -I$(LUADIR)/src
LUA_AR_LIBS = $(LUADIR)/src/liblua.a

INCLUDES += -I$(LUADIR)/src -I$(LUADIR)/etc

ifeq ($(PLATFORM), mingw)
CFLAGS += -I/usr/include
LIBS += -L/usr/lib
endif

DISP_SRC_FILES = vector_float.c disp-utils.c data-table.c dispers.c disp-ho.c disp-library.c disp-table.c lua-disp.c

DISP_OBJ_FILES := $(DISP_SRC_FILES:%.c=%.o)
DEP_FILES      := $(DISP_SRC_FILES:%.c=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

COMPILE = $(CC) --std=c99 $(CFLAGS) $(LUA_CFLAGS) $(DEFS) $(INCLUDES)

GSL_LIBS = -lgsl -lgslcblas -lm
LIBS += $(GSL_LIBS)

ifeq ($(PLATFORM), mingw)
TARGETS = disp.dll
else
TARGETS = disp.so
endif

all: $(TARGETS)

ifeq ($(PLATFORM), mingw)

disp.dll: $(DISP_OBJ_FILES) $(LUA_AR_LIBS)
	$(CC) -O -shared -Wl,--enable-auto-import -o $@ $(DISP_OBJ_FILES) $(LUA_AR_LIBS) $(LIBS)

else

disp.so: $(DISP_OBJ_FILES)  $(LUA_AR_LIBS)
	$(CC) -shared -o .libs/libluadisp.so $(DISP_OBJ_FILES) $(LUA_AR_LIBS) $(LIBS)
	ln -sf ./.libs/libluadisp.so $@

endif

%.o: %.c
	@echo $(COMPILE) -c $<
	$(COMPILE) -Wp,-MMD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

.PHONY: clean all

clean:
	$(RM) *.o *.lo *.la *.so *.dll $(TARGETS)

-include $(DEP_FILES)

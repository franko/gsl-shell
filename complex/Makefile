
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

INCLUDES += -I$(GSH_BASE_DIR) -I$(LUADIR)/src

CMPL_SRC_FILES = lcomplex.c

CMPL_OBJ_FILES := $(CMPL_SRC_FILES:%.c=%.o)

DEP_FILES := $(CMPL_SRC_FILES:%.c=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

COMPILE = $(CC) $(CFLAGS) $(DEFS) $(INCLUDES) 

TARGETS = liblcomplex.a

all: $(TARGETS)

liblcomplex.a: $(CMPL_OBJ_FILES)
	$(AR) $@ $?
	$(RANLIB) $@

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

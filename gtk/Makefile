
GSH_BASE_DIR = ..

include $(GSH_BASE_DIR)/makeconfig
include $(GSH_BASE_DIR)/make-system-detect
include $(GSH_BASE_DIR)/makeflags
include $(GSH_BASE_DIR)/makepackages
include $(GSH_BASE_DIR)/makedefs

GTK_CFLAGS := $(shell pkg-config gtk+-2.0 --cflags)
GTK_LIBS := $(shell pkg-config gtk+-2.0 --libs)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

INCLUDES += $(GTK_CFLAGS) -I$(GSH_BASE_DIR) -I$(GSH_BASE_DIR)/luajit2/src
LDFLAGS += $(GTK_LIBS) -lpthread

DEFS += -D_REENTRANT

COMPILE = $(CC) $(CFLAGS) $(DEFS) $(INCLUDES)
LINK_EXE = $(CC) $(LDFLAGS)

C_SRC_FILES = gsl-shell-gtk.c gsl-shell-interp.c
OBJ_FILES = $(C_SRC_FILES:%.c=%.o)

gsl-shell-gtk: $(OBJ_FILES)
	$(LINK_EXE) -o gsl-shell-gtk $(OBJ_FILES) ../luajit2/src/libluajit.a $(GTK_LIBS) -lgsl -lgslcblas -lm -lsupc++

include $(GSH_BASE_DIR)/makerules

-include $(DEP_FILES)

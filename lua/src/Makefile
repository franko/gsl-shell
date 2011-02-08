# makefile for building Lua
# see ../INSTALL for installation instructions
# see ../Makefile and luaconf.h for further customization

# == CHANGE THE SETTINGS BELOW TO SUIT YOUR ENVIRONMENT =======================

GSH_BASE_DIR = ../..

include $(GSH_BASE_DIR)/makeconfig
include $(GSH_BASE_DIR)/makeflags
include $(GSH_BASE_DIR)/makedefs

LUA_A = liblua.a
RM = rm -f
LIBS = -lm

# == END OF USER SETTINGS. NO NEED TO CHANGE ANYTHING BELOW THIS LINE =========

CORE_C_SRC = lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c \
	lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c ltm.c  \
	lundump.c lvm.c lzio.c
LIB_C_SRC = lauxlib.c lbaselib.c ldblib.c liolib.c lmathlib.c loslib.c ltablib.c \
	lstrlib.c loadlib.c linit.c

LUA_T  = lua$(EXE_EXT)
LUAC_T = luac$(EXE_EXT)

ifeq ($(strip $(PLATFORM)), linux)
  CFLAGS += -DLUA_USE_LINUX
  LIBS += -Wl,-E -ldl -lreadline -lhistory -lncurses
endif

LUA_O=	lua.o
LUAC_O=	luac.o print.o

CORE_O = $(CORE_C_SRC:%.c=%.o)
LIB_O = $(LIB_C_SRC:%.c=%.o)

DEP_FILES := $(CORE_C_SRC:%.c=.deps/%.P) $(LIB_C_SRC:%.c=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

ALL_O= $(CORE_O) $(LIB_O) $(LUA_O) $(LUAC_O)
ALL_T= $(LUA_A) $(LUA_T) $(LUAC_T)
ALL_A= $(LUA_A)

COMPILE = $(CC) $(CFLAGS) $(LUA_CFLAGS) $(DEFS) $(INCLUDES)
CXXCOMPILE = $(CXX) $(CXXFLAGS) -c

all:	$(ALL_A)

o:	$(ALL_O)

a:	$(ALL_A)

$(LUA_A): $(CORE_O) $(LIB_O)
	$(AR) $@ $?
	$(RANLIB) $@

$(LUA_T): $(LUA_O) $(LUA_A)
	$(CC) -o $@ $(LDFLAGS) $(LUA_O) $(LUA_A) $(LIBS)

$(LUAC_T): $(LUAC_O) $(LUA_A)
	$(CC) -o $@ $(LDFLAGS) $(LUAC_O) $(LUA_A) $(LIBS)

clean:
	$(RM) $(ALL_T) $(ALL_O)

depend:
	@$(CC) $(CFLAGS) -MM l*.c print.c

include $(GSH_BASE_DIR)/makerules

# list targets that do not create files (but not all makes understand .PHONY)
.PHONY: all default o a clean depend

-include $(DEP_FILES)

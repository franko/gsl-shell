
GSH_BASE_DIR = ../..

include $(GSH_BASE_DIR)/makeconfig
include $(GSH_BASE_DIR)/make-system-detect
include $(GSH_BASE_DIR)/makepackages
include $(GSH_BASE_DIR)/makedefs

INCLUDES += -I. $(GSL_INCLUDES)
LIBS += $(GSL_LIBS)

CFLAGS = -march=i686 -O2 -fomit-frame-pointer

ifeq ($(HOST_SYS),Darwin)
  LINK_EXE = $(CXX) $(LDFLAGS)
else
  LINK_EXE = $(CC) $(LDFLAGS)
endif

ode-benchmark.exe: ode-benchmark.o
	@echo Linking $@
	@$(LINK_EXE) -o $@ $< $(LIBS)

ode-benchmark-rk8pd.exe: ode-benchmark-rk8pd.o
	@echo Linking $@
	@$(LINK_EXE) -o $@ $< $(LIBS)

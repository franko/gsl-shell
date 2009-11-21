
CXX = g++
CXXFLAGS = -g -Wall

AGG_CFLAGS = -I/usr/include/agg2
AGG_LIBS = -lagg -laggplatformX11 -lX11

CXXCOMPILE = $(CXX) $(CXXFLAGS)

AGG_HOME = /home/francesco/src/agg-2.5

AGG_TRANS_AFFINE = $(AGG_HOME)/src/agg_trans_affine.o

SRC_FILES = utils.cpp cplot.cpp units_cplot.cpp main.cpp

OBJ_FILES := $(SRC_FILES:%.cpp=%.o)

.PHONY: clean all

all: test.exe

%.o: %.cpp
	$(CXXCOMPILE) -c $< $(AGG_CFLAGS)

test.exe: $(OBJ_FILES)
	$(CXX) -o $@ $(OBJ_FILES) $(AGG_TRANS_AFFINE) $(AGG_LIBS)

clean:
	rm *.o *.exe

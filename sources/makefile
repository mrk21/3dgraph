# Project: 3dGraph
# Makefile created by Dev-C++ 4.9.9.2

CPP     = g++.exe
CC      = gcc.exe
WINDRES = windres.exe

RES     = 3dGraph.res
OBJ     = 3dGraph.o $(RES)
LINKOBJ = 3dGraph.o $(RES)

LIBS    = -L"C:/MinGW/lib" -mwindows C:/MinGW/lib/libopengl32.a
INCS    = -I"C:/MinGW/include"
CXXINCS = -I"C:/MinGW/lib/gcc/mingw32/3.4.5/include" -I"C:/MinGW/include/c++/3.4.5/backward" -I"C:/MinGW/include/c++/3.4.5/mingw32" -I"C:/MinGW/include/c++/3.4.5" -I"C:/MinGW/include" -I"C:/lib/boost/1.34.1"

BIN      = 3dGraph.exe
CXXFLAGS = $(CXXINCS)
CFLAGS   = $(INCS)
RM       = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before 3dGraph.exe all-after


clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o "3dGraph.exe" $(LIBS)

3dGraph.o: 3dGraph.cpp
	$(CPP) -c 3dGraph.cpp -o 3dGraph.o $(CXXFLAGS)

3dGraph.res: Resource/Form.rc
	$(WINDRES) --language=0411 -i Resource/Form.rc --input-format=rc -o 3dGraph.res -O coff

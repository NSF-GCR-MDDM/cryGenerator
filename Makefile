# ---- CONFIG ----
CXX := $(shell root-config --cxx)
CXXFLAGS := -O2 -Wall $(shell root-config --cflags)

INCLUDE := -I$(shell root-config --incdir)
LIBDIRS := -L$(shell root-config --libdir)
ROOTLIBS := $(shell root-config --libs)

CRY_INC := -I$(CRYPATH)
CRY_LIB := $(CRYPATH)/libCRY.a

TARGET := cryGenerator
SRC := cryGenerator.cc

# ---- BUILD ----
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(CRY_INC) $(SRC) $(CRY_LIB) $(LIBDIRS) $(ROOTLIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)

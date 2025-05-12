# CRY Makefile with ROOT compatibility and archive index fix

CXX := $(shell root-config --cxx)
CPPFLAGS := -O2 -Wall $(shell root-config --cflags)
AR := ar
RANLIB := ranlib

SRC := $(wildcard *.cc)
OBJ := $(SRC:%.cc=%.o)
DEP := $(SRC:%.cc=.%.d)
LIB := libCRY.a

default: $(LIB)

$(LIB): $(OBJ)
	$(AR) cr $(LIB) $(OBJ)
	$(RANLIB) $(LIB)
	@echo "Done making $(LIB)"

%.o: %.cc
	$(CXX) $(CPPFLAGS) -c $< -o $@

.%.d: %.cc
	$(CXX) $(CPPFLAGS) -MM -MG -MT "$@ $*.o" -MF $@ $<

clean:
	rm -f *.o .*.d $(LIB)

.PHONY: clean
-include $(DEP)

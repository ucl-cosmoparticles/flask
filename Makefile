# Include local settings for specific computers
-include Makefile.local


#####################
### Configuration ###
#####################

# Do not change. Use Makefile.local instead.

# Flask directory structure:
BIN = bin
OBJ = obj
SRC = src

# Default variables:
ifeq ($(HEALPIX),)
HEALPIX_DATA ?=
HEALPIX_CFLAGS ?=
HEALPIX_LDFLAGS ?=
else
HEALPIX_DATA ?= $(HEALPIX)/data
HEALPIX_CFLAGS ?= -I$(HEALPIX)/include
HEALPIX_LDFLAGS ?= -L$(HEALPIX)/lib -Wl,-rpath,$(HEALPIX)/lib
endif
HEALPIX_LDLIBS ?= -lhealpix_cxx -lsharp
GSL_CFLAGS ?=
GSL_LDFLAGS ?=
GSL_LDLIBS ?= -lgsl -lgslcblas
CFITSIO_CFLAGS ?=
CFITSIO_LDFLAGS ?=
CFITSIO_LDLIBS ?= -lcfitsio
OMP_CFLAGS ?= -fopenmp
OMP_LDFLAGS ?=
OMP_LDLIBS ?= -lgomp

# Compiler flags:
CXXFLAGS += $(HEALPIX_CFLAGS) $(GSL_CFLAGS) $(CFITSIO_CFLAGS) $(OMP_CFLAGS)

# Linker flags:
LDFLAGS += $(HEALPIX_LDFLAGS) $(GSL_LDFLAGS) $(CFITSIO_LDFLAGS) $(OMP_LDFLAGS)

# Libraries to link:
LDLIBS += $(HEALPIX_LDLIBS) $(GSL_LDLIBS) $(CFITSIO_LDLIBS) $(OMP_LDLIBS)

# Get code version
FLASKCOMMIT := $(shell git rev-parse --short HEAD 2>/dev/null)
ifeq ($(FLASKCOMMIT),)
  FLASKCOMMIT = missing
endif
CXXFLAGS += -DFLASKCOMMIT=\"$(FLASKCOMMIT)\"

# Check if HEALPix includes are directly accessible
ifeq ($(shell echo '\#include <healpix_base.h>' | $(CXX) -E -x c++ $(CXXFLAGS) - 2>&1 > /dev/null; echo $$?),0)
CXXFLAGS += -DFLASK_NO_HEALPIX_CXX_INCLUDE_DIR
endif

# Defines:
CXXFLAGS += -DUSEXCOMPLEX=0
CXXFLAGS += -DUSEMAP2TGA=0
CXXFLAGS += -DHEALPIX_DATA=\"$(HEALPIX_DATA)\"


#############################
### Compiling intructions ###
#############################

# General instructions:

.PHONY: all clean test

all: $(BIN)/flask $(BIN)/GenStarMask $(BIN)/XiCalculator $(BIN)/Dens2KappaCls

clean:
	$(RM) $(BIN)/*
	$(RM) $(OBJ)/*

test:


# Dependencies:

SOURCES = $(wildcard $(SRC)/*.cpp)
DEPEND = $(SOURCES:$(SRC)/%.cpp=$(OBJ)/%.d)

-include $(DEPEND)

$(OBJ)/%.d: $(SRC)/%.cpp
	$(info depend $<)
	@$(CXX) $(CXXFLAGS) -MM -MF$@ -MT$(@:.d=.o) -MT$@ $<


# Executables:

$(BIN)/Dens2KappaCls: $(OBJ)/Dens2KappaCls.o $(OBJ)/ParameterList.o $(OBJ)/Utilities.o $(OBJ)/Cosmology.o $(OBJ)/FieldsDatabase.o $(OBJ)/Integral.o $(OBJ)/interpol.o $(OBJ)/Spline.o
	$(info link $@)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(BIN)/XiCalculator: $(OBJ)/XiCalculator.o $(OBJ)/Utilities.o
	$(info link $@)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(BIN)/GenStarMask: $(OBJ)/GenStarMask.o
	$(info link $@)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(BIN)/flask: $(OBJ)/flask.o $(OBJ)/ParameterList.o $(OBJ)/Utilities.o $(OBJ)/gsl_aux.o $(OBJ)/s2kit10_naive.o $(OBJ)/interpol.o $(OBJ)/flask_aux.o $(OBJ)/Cosmology.o $(OBJ)/Integral.o $(OBJ)/GeneralOutput.o $(OBJ)/SelectionFunc.o $(OBJ)/RegularizeCov.o $(OBJ)/ClProcessing.o $(OBJ)/Maximize.o $(OBJ)/fitsfunctions.o $(OBJ)/lognormal.o $(OBJ)/FieldsDatabase.o $(OBJ)/Spline.o
	$(info link $@)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)


# Objects:

$(OBJ)/%.o: $(SRC)/%.cpp
	$(info compile $<)
	$(CXX) -c $(CXXFLAGS) -fPIC $< -o $@

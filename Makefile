# System architecture
SYSTEM     = x86-64_linux
#SYSTEM     = x86-64_sles10_4.1

# Static library format for Cplex
LIBFORMAT  = static_pic

# Source code folder
SRC	= src
INCLUDE = include

# Machine hostname
MACHINE = $(shell hostname)

# Library type(STATIC or DYNAMIC)
MERGE = DYNAMIC

##### Folders
# Temp folders
TMP_KC = ./tmp/KC
TMP_STATIC = ./tmp/lib/static
# Perm folders
DAT_DOXYFILE = ./dat/doxyfile
DAT_INSTANCES = ./dat/instances
DAT_LP_MODELS = ./dat/lp_models
DAT_RESULTS = ./dat/results


# Cplex directory
CPLEXDIR	  = /opt/ibm/ILOG/CPLEX_Studio1261/cplex
#CPLEXDIR	  = /opt/ibm/ILOG/CPLEX_Studio1251/cplex

# Concert directory
CONCERTDIR	  = /opt/ibm/ILOG/CPLEX_Studio1261/concert
#CONCERTDIR	  = /opt/ibm/ILOG/CPLEX_Studio1251/concert

# Compiler
CCC = g++

# Compilation parameters (Add afterward: --coverage -pg -ftree-vectorize -mfpmath=sse -march=native)
CCOPT = -std=gnu++0x -O3 -ftree-vectorize -mfpmath=sse -march=native -flto -g -m64 -fPIC -fexceptions -DNDEBUG -DIL_STD

# Cplex static libraries directory
CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

# Concert static libraries directory
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

# Include libraries identifiers
CCLNFLAGS = -L$(CPLEXLIBDIR) -lilocplex -lcplex -L$(CONCERTLIBDIR) -lconcert -lm -pthread

# Cplex header's directory
CPLEXINCDIR   = $(CPLEXDIR)/include

# Concert header's directory
CONCERTINCDIR = $(CONCERTDIR)/include

# Header's include path
CCFLAGS = $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) #-lemon

# Executable name
CPP_EX = kc-coloring

# Compiling
all:
	mkdir -p $(TMP_KC)
	mkdir -p $(TMP_STATIC)
	mkdir -p $(DAT_DOXYFILE)
	mkdir -p $(DAT_INSTANCES)
	mkdir -p $(DAT_LP_MODELS)
	mkdir -p $(DAT_RESULTS)
	make -j8 $(CPP_EX);

# Executing
execute: $(CPP_EX)
	./$(CPP_EX)

# Cleaning
clean:
	/bin/rm -rf $(CPP_EX)
	/bin/rm -rf ./tmp
	/bin/rm -rf ./dat
	/bin/rm -rf *.lp
	/bin/rm -rf cutset.txt
	/bin/rm -rf results.csv

########################## GENERATING OBJECT's ######################################################

# CONFIGURATION - INSTANCES
$(TMP_KC)/instance.o: $(SRC)/instance.cpp $(INCLUDE)/instance.h
	$(CCC) -c $(CCFLAGS) $(SRC)/instance.cpp -o $(TMP_KC)/instance.o

# STRUCTURE - CHRONO
$(TMP_KC)/FWChrono.o: $(SRC)/FWChrono.cpp $(INCLUDE)/FWChrono.h
	$(CCC) -c $(CCFLAGS) $(SRC)/FWChrono.cpp -o $(TMP_KC)/FWChrono.o

# EXACT
$(TMP_KC)/callbacks.o: $(SRC)/callbacks.cpp $(INCLUDE)/callbacks.h
	$(CCC) -c $(CCFLAGS) $(SRC)/callbacks.cpp -o $(TMP_KC)/callbacks.o
$(TMP_KC)/model.o: $(SRC)/model.cpp $(INCLUDE)/model.h
	$(CCC) -c $(CCFLAGS) $(SRC)/model.cpp -o $(TMP_KC)/model.o
# $(TMP_KC)/solver.o: $(SRC)/solver.cpp $(INCLUDE)/solver.h
# 	$(CCC) -c $(CCFLAGS) $(SRC)/solver.cpp -o $(TMP_KC)/solver.o

# MAIN
$(TMP_KC)/main.o: $(SRC)/main.cpp
	$(CCC) -c $(CCFLAGS) $(SRC)/main.cpp -o $(TMP_KC)/main.o

########################## LINKANDO TUDO ########################################################
$(CPP_EX): $(TMP_KC)/callbacks.o $(TMP_KC)/model.o $(TMP_KC)/instance.o $(TMP_KC)/FWChrono.o $(TMP_KC)/main.o
	$(CCC)  $(CCFLAGS) $(TMP_KC)/callbacks.o $(TMP_KC)/model.o $(TMP_KC)/instance.o $(TMP_KC)/FWChrono.o $(TMP_KC)/main.o -L$(TMP_STATIC) -o $(CPP_EX) $(CCLNFLAGS)

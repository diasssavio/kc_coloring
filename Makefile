# System and architecture infos
PLATFORM = linux64
GUROBI_HOME = /opt/gurobi702/$(PLATFORM)
INC      = $(GUROBI_HOME)/include/
CC       = gcc
CPP      = g++
CARGS    = -std=gnu++0x -O3 -ftree-vectorize -mfpmath=sse -march=native -flto -g -m64 -fPIC -fexceptions -DNDEBUG -DIL_STD
CLIB     = -L$(GUROBI_HOME)/lib/ -lgurobi70 -lm
# CPPLIB   = -L$(GUROBI_HOME)/lib/ -lgurobi_c++ -lgurobi65

SRC      = src
INCLUDE  = include

# Temp folders
TMP_KC   = ./tmp/KC
TMP_STATIC = ./tmp/lib/static

# Header's include path
CCFLAGS  = $(CARGS) -I$(INC)
# CCFLAGS  = $(CARGS) -I$(INC) $(CLIB)

# Executable name
CPP_EX   = kc_coloring

# Compiling
all:
	mkdir -p $(TMP_KC)
	make -j8 $(CPP_EX);

# Executing
execute: $(CPP_EX)
	./$(CPP_EX)

# Cleaning
clean:
	/bin/rm -rf $(CPP_EX)
	/bin/rm -rf ./tmp
	/bin/rm -rf *.lp
	/bin/rm -rf *.log
	/bin/rm -rf cutset.txt
	/bin/rm -rf results.csv

########################## GENERATING OBJECT's ######################################################

# UTILS
$(TMP_KC)/utils.o: $(SRC)/utils.cpp $(INCLUDE)/utils.h
	$(CPP) -c $(CCFLAGS) $(SRC)/utils.cpp -o $(TMP_KC)/utils.o

# CONFIGURATION - INSTANCES
$(TMP_KC)/instance.o: $(SRC)/instance.cpp $(INCLUDE)/instance.h
	$(CPP) -c $(CCFLAGS) $(SRC)/instance.cpp -o $(TMP_KC)/instance.o

# STRUCTURE - CHRONO
$(TMP_KC)/FWChrono.o: $(SRC)/FWChrono.cpp $(INCLUDE)/FWChrono.h
	$(CPP) -c $(CCFLAGS) $(SRC)/FWChrono.cpp -o $(TMP_KC)/FWChrono.o

# MAIN
$(TMP_KC)/main.o: $(SRC)/main.cpp
	$(CPP) -c $(CCFLAGS) $(SRC)/main.cpp -o $(TMP_KC)/main.o

########################## LINKANDO TUDO ########################################################
$(CPP_EX): $(TMP_KC)/utils.o $(TMP_KC)/instance.o $(TMP_KC)/FWChrono.o $(TMP_KC)/main.o
	$(CPP) $(CCFLAGS) $(TMP_KC)/utils.o $(TMP_KC)/instance.o $(TMP_KC)/FWChrono.o $(TMP_KC)/main.o -L$(TMP_STATIC) -o $(CPP_EX) $(CLIB)

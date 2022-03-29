ELF  = ecgrep
LIB  = 

SDIR = src
IDIR = include
ODIR = obj
BDIR = bin

SRC  = ecgrep.cpp matchfinder.cpp
DEP  = matchfinder.hpp

$(ELF): $(patsubst %,$(ODIR)/%.o,$(basename $(SRC)))
	mkdir -p $(BDIR)
	g++ $^ $(patsubst %,-l%,$(LIB)) -o $(BDIR)/$@

$(ODIR)/%.o: $(SDIR)/%.cpp $(patsubst %,$(IDIR)/%,$(DEP))
	g++ -Wall -std=c++20 -O0 -g -I$(IDIR) -c $< -o $@

all: options $(ELF)

clean:
	rm -rf $(ODIR)/*.o $(BDIR)/

options:
	@echo "ELF  = ${ELF}"
	@echo "LIB  = ${LIB}"
	@echo "SDIR = ${SDIR}" 
	@echo "IDIR = ${IDIR}" 
	@echo "ODIR = ${ODIR}" 
	@echo "BDIR = ${BDIR}" 
	@echo "SRC  = ${SRC}"
	@echo "DEP  = ${DEP}"

.PHONY: all clean options

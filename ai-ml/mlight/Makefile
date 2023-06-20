# Install Armadillo first: http://arma.sourceforge.net/download.html
#
include Macros.mk

TOPDIR = ..

TESTS = $(patsubst %.cpp,%,$(wildcard *Test.cpp))
BINARIES = $(patsubst %.cpp,%,$(wildcard *Main.cpp))

LIBS_SOURCES := $(filter-out %Test.cpp,$(wildcard *.cpp))
LIBS_SOURCES := $(filter-out %Main.cpp,$(LIBS_SOURCES))
LIBS = $(patsubst %.cpp,%.o,$(LIBS_SOURCES))

all: test $(BINARIES)

$(TESTS): $(LIBS) $(patsubst %,%.o,$(TESTS))
	# Will have a recipe for each target in $(TESTS)
	$(CXX) -o $@ $(LIBS) $@.cpp $(LDFLAGS) -ldl $(CXXFLAGS)

$(BINARIES): $(LIBS) $(patsubst %,%.o,$(BINARIES))
	$(CXX) -o $@ $@.cpp $(LIBS) $(LDFLAGS) -ldl $(CXXFLAGS)

test: $(TESTS)
	echo '$^' | xargs -d' ' -I'{}' echo "./{}" | bash -

.PHONY: clean

clean:
	rm -f $(LIBS) $(TESTS) $(BINARIES)


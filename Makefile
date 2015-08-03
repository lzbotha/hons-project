#
# TODO: Move `libmongoclient.a` to /usr/local/lib so this can work on production servers
#

CXX := g++ # This is the main compiler

SRCDIR := src
BUILDDIR := build
TARGET := bin/runner

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CPPFLAGS := -g -std=c++11 $(shell root-config --cflags)
LIB := -lcppunit -lassimp $(shell root-config --libs)
INC := -g $(shell root-config --ldflags)

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@echo " $(CXX) $^ -o $(TARGET) $(LIB)"; $(CXX) $^ -o $(TARGET) $(LIB)


# TODO: make this depend on the header files as well
$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CXX) $(CPPFLAGS) $(INC) -c -o $@ $<"; $(CXX) $(CPPFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

# Tests
# TODO MAKE THESE AUTO DISCOVER AND STUFF
tester:
	$(CXX) $(CPPFLAGS) test/tester.cpp $(INC) $(LIB) -o bin/tester

# Spikes
ticket:
	$(CXX) $(CPPFLAGS) spikes/ticket.cpp $(INC) $(LIB) -o bin/ticket

.PHONY: clean


# http://hiltmon.com/blog/2013/07/03/a-simple-c-plus-plus-project-structure/
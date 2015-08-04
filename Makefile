#
# TODO: Move `libmongoclient.a` to /usr/local/lib so this can work on production servers
#

CXX := g++ # This is the main compiler

SRCDIR := src
TESTDIR := test
BUILDDIR := build
TARGET := bin/runner
TESTTARGET := bin/test_runner

SRCEXT := cpp

SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

TESTSOURCES := $(shell find $(TESTDIR) -type f -name *.$(SRCEXT))
TESTOBJECTS := $(patsubst $(TESTDIR)/%,$(TESTDIR)/%,$(TESTSOURCES:.$(SRCEXT)=.o))

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

$(TESTDIR)/%.o: $(TESTDIR)/%.$(SRCEXT)
	@echo " $(CXX) $(CPPFLAGS) $(INC) -c -o $@ $<"; $(CXX) $(CPPFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)
	@echo " $(RM) $(TESTDIR)/*.o"; $(RM) $(TESTDIR)/*.o

# Tests
tests: $(TESTOBJECTS)
	$(CXX) $^ build/utils.o -o $(TESTTARGET) $(LIB)
	

.PHONY: clean


# http://hiltmon.com/blog/2013/07/03/a-simple-c-plus-plus-project-structure/
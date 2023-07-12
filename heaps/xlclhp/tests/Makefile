BUILD ?= ./build
LIBNAME ?= xlhtest

# Predefined folders

SRCD := src

# Temporary folders

LIBD := $(BUILD)/lib
BIND := $(BUILD)/bin
OBJD := $(BUILD)/obj/heaps
DIRS := $(LIBD) $(BIND) $(OBJD)

# Library and test exe path base

LIBTGT := $(LIBD)/lib$(LIBNAME)
EXETGT := $(BIND)/xtc-test-xlclhp

# Source list

SRCS := cu_main cu_tests_xlh
SRCPPS := cxx_tests_xlh

# Tool chain base flags

FLAGS := -Wall -Wextra -Werror -Wpedantic --pedantic-errors
FLAGS += -I ../../xgnrhp/src -I ../src
CFLAGS += $(FLAGS) -std=c11
CXXFLAGS += $(FLAGS) -std=c++11
LDFLAGS += -fPIC -lcunit -L$(LIBD)

# Main target

all: dirs $(LIBTGT).a $(LIBTGT)d.a $(EXETGT) $(EXETGT)-d

# Dependencies management

DEPS := $(SRCS:%=$(OBJD)/%.d) $(SRCPPS:%=$(OBJD)/%.d)

$(OBJD)/%.d: $(SRCD)/%.c $(SRCD)/%.cpp | $(OBJD)
	@$(CC) -MM -MP -MT $(OBJD)/$(basename $(<F)).o -MT $(OBJD)/$(basename $(<F))-g.o $(CFLAGS) $< > $@

# Target specificities

$(EXETGT): CFLAGS += -O2 -DNDEBUG=1
$(EXETGT): CXXFLAGS += -O2 -DNDEBUG=1
$(EXETGT): LDFLAGS += -l$(LIBNAME)
$(EXETGT)-d: CFLAGS += -g -D__DEBUG
$(EXETGT)-d: CXXFLAGS += -g -D__DEBUG
$(EXETGT)-d: LDFLAGS += -l$(LIBNAME)d

# Object files build rules

$(OBJD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJD)/%-g.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJD)/%.o: $(SRCD)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJD)/%-g.o: $(SRCD)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Executable targets

$(EXETGT): $(SRCS:%=$(OBJD)/%.o) $(SRCPPS:%=$(OBJD)/%.o)
	$(CXX) $^ $(LDFLAGS) -o $@

$(EXETGT)-d: $(SRCS:%=$(OBJD)/%-g.o)  $(SRCPPS:%=$(OBJD)/%-g.o)
	$(CXX) $^ $(LDFLAGS) -o $@

# Library targets

$(LIBTGT).a $(LIBTGT)d.a:
	@$(MAKE) -C .. BUILD=./tests/$(BUILD) LIBNAME=$(LIBNAME)

# Other targets

clean:
	@-$(RM) -r $(BUILD)

dirs: $(DIRS)

$(DIRS):
	@mkdir -p $@

.PHONY: all clean dirs

# Dependencies

-include $(DEPS)

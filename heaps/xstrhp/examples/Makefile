BUILD ?= ./build
LIBNAME ?= xshexample

# Predefined folders

SRCCD := src-c
SRCCPPD := src-cpp

# Temporary folders

LIBD := $(BUILD)/lib
BIND := $(BUILD)/bin
OBJD := $(BUILD)/obj/heaps
DIRS := $(LIBD) $(BIND) $(OBJD)

# Library and test exe path base

LIBTGT := $(LIBD)/lib$(LIBNAME)
EXETGTC := $(BIND)/xtc-xmp-xstrhp-c
EXETGTCPP := $(BIND)/xtc-xmp-xstrhp-cpp

# Source list

SRCCS := xmp_xsh_main xmp_xsh_static xmp_xsh_prealloc
SRCCPPS := xmp_xsh_main-cpp xmp_xsh_myclass

# Tool chain base flags

FLAGS = -Wall -Wextra -Werror -Wpedantic --pedantic-errors
FLAGS += -I ../../xgnrhp/src -I ../src
CFLAGS += $(FLAGS) -std=c11
CXXFLAGS += $(FLAGS) -std=c++11
LDFLAGS += -fPIC -l$(LIBNAME) -L$(LIBD)

# Main target

all: dirs $(LIBTGT).a $(LIBTGT)d.a $(EXETGTC) $(EXETGTC)-d $(EXETGTCPP) $(EXETGTCPP)-d

# Dependencies management

DEPS := $(SRCCS:%=$(OBJD)/%.d) $(SRCCPPS:%=$(OBJD)/%.d)

$(OBJD)/%.d: $(SRCCD)/%.c | $(OBJD)
	@$(CC) -MM -MP -MT $(OBJD)/$(basename $(<F)).o -MT $(OBJD)/$(basename $(<F))-g.o $(CFLAGS) $< > $@

$(OBJD)/%.d: $(SRCCPPD)/%.c | $(OBJD)
	@$(CXX) -MM -MP -MT $(OBJD)/$(basename $(<F)).o -MT $(OBJD)/$(basename $(<F))-g.o $(CXXFLAGS) $< > $@

# Target specificities

$(EXETGTC): CFLAGS += -O2 -DNDEBUG=1
$(EXETGTC)-d: CFLAGS += -g -D__DEBUG
$(EXETGTCPP): CXXFLAGS += -O2 -DNDEBUG=1
$(EXETGTCPP)-d: CXXFLAGS += -g -D__DEBUG

# Object files build rules

$(OBJD)/%.o: $(SRCCD)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJD)/%-g.o: $(SRCCD)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJD)/%.o: $(SRCCPPD)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJD)/%-g.o: $(SRCCPPD)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Executable targets

$(EXETGTC): $(SRCCS:%=$(OBJD)/%.o)
	$(CC) $^ $(LDFLAGS) -lpthread -o $@

$(EXETGTC)-d: $(SRCCS:%=$(OBJD)/%-g.o)
	$(CC) $^ $(LDFLAGS) -lpthread -o $@

$(EXETGTCPP): $(SRCCPPS:%=$(OBJD)/%.o)
	$(CXX) $^ $(LDFLAGS) -o $@

$(EXETGTCPP)-d: $(SRCCPPS:%=$(OBJD)/%-g.o)
	$(CXX) $^ $(LDFLAGS) -o $@

# Library targets

$(LIBTGT).a $(LIBTGT)d.a:
# Need generic implementation as well
	@$(MAKE) -C .. BUILD=./examples/$(BUILD) LIBNAME=$(LIBNAME)

# Other targets

clean:
	@-$(RM) -r $(BUILD)

dirs: $(DIRS)

$(DIRS):
	@mkdir -p $@

.PHONY: all clean dirs

# Dependencies

-include $(DEPS)

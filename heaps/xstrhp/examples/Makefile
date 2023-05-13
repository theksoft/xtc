BUILD ?= ./build
LIBNAME ?= xshexample

# Predefined folders

SRCD := src

# Temporary folders

LIBD := $(BUILD)/lib
BIND := $(BUILD)/bin
OBJD := $(BUILD)/obj/heaps
DIRS := $(LIBD) $(BIND) $(OBJD)

# Library and test exe path base

LIBTGT := $(LIBD)/lib$(LIBNAME)
EXETGT := $(BIND)/xtc-xmp-xstrhp

# Source list

SRCS := xmp_main_xsh xmp_static_xsh xmp_prealloc_xsh

# Tool chain base flags

CFLAGS += -Wall -Wextra -Werror -Wpedantic --pedantic-errors -std=c11
CFLAGS += -I ../../xgnrhp/src -I ../src
LDFLAGS += -fPIC -l$(LIBNAME) -L$(LIBD) -lpthread

# Main target

all: dirs $(LIBTGT).a $(LIBTGT)d.a $(EXETGT) $(EXETGT)-d

# Dependencies management

DEPS := $(SRCS:%=$(OBJD)/%.d)

$(OBJD)/%.d: $(SRCD)/%.c | $(OBJD)
	@$(CC) -MM -MP -MT $(OBJD)/$(basename $(<F)).o -MT $(OBJD)/$(basename $(<F))-g.o $(CFLAGS) $< > $@

# Target specificities

$(EXETGT): CFLAGS += -O2 -DNDEBUG=1
$(EXETGT)-d: CFLAGS += -g -D__DEBUG

# Object files build rules

$(OBJD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJD)/%-g.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Executable targets

$(EXETGT): $(SRCS:%=$(OBJD)/%.o)
	$(CC) $^ $(LDFLAGS) -o $@

$(EXETGT)-d: $(SRCS:%=$(OBJD)/%-g.o)
	$(CC) $^ $(LDFLAGS) -o $@

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

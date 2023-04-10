BUILD ?= build
LIBNAME ?= xgnrhp

# Predefined folders

SRCD := src

# Temporary folders

LIBD := $(BUILD)/lib
BIND := $(BUILD)/bin
OBJD := $(BUILD)/obj/heaps
DIRS := $(LIBD) $(BIND) $(OBJD)

# Library path base

LIBTGT := $(LIBD)/lib$(LIBNAME)

# Source list

SRCS := xtc_gnrhp

# Tool chain base flags

CFLAGS += -Wall -Wextra -Werror -Wpedantic --pedantic-errors -std=c11
CFLAGS += -I ../src
ARFLAGS := rcs

# Main target
all: dirs $(LIBTGT).a $(LIBTGT)d.a
	@echo "..... XTC COMMON HEAPS BUILD DONE / lib = $(LIBNAME) ....."

tests: dirs $(LIBTGT).a $(LIBTGT)d.a
	@echo "..... XTC COMMON HEAPS TEST BUILD DONE / test = $(TSTNAME) ....."

examples: dirs $(LIBTGT).a $(LIBTGT)d.a
	@echo "..... XTC COMMON HEAPS EXAMPLE BUILD DONE / example = $(XMPNAME) ....."

# Dependencies management

DEPS := $(SRCS:%=$(OBJD)/%.d)

$(OBJD)/%.d: $(SRCD)/%.c | $(OBJD)
	@$(CC) -MM -MP -MT $(OBJD)/$(basename $(<F)).o -MT $(OBJD)/$(basename $(<F))-g.o $(CFLAGS) $< > $@

# Target specificities

$(LIBTGT).a: CFLAGS += -O2 -DNDEBUG=1
$(LIBTGT)d.a: CFLAGS += -g -D__DEBUG

# CompilationObject files build rules

$(OBJD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJD)/%-g.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Library targets

$(LIBTGT).a: $(SRCS:%=$(OBJD)/%.o)
	$(AR) $(ARFLAGS) $@ $^

$(LIBTGT)d.a: $(SRCS:%=$(OBJD)/%-g.o)
	$(AR) $(ARFLAGS) $@ $^

# Other targets

clean: $(PKGS)
	@-$(RM) -r $(BUILD)

dirs: $(DIRS)

$(DIRS):
	@mkdir -p $@

.PHONY: all clean tests examples dirs $(DIRS)

# Dependencies

-include $(DEPS)

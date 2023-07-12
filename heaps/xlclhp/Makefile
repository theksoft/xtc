BUILD ?= ./build
LIBNAME ?= xtclclhp

PKGBLD := ../$(BUILD)

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

SRCS := xtc_lclhp

# Tool chain base flags

CFLAGS += -Wall -Wextra -Werror -Wpedantic --pedantic-errors -std=c11
CFLAGS += -I ../xgnrhp/src
ARFLAGS := rcs

# Main target
all: dirs $(LIBTGT).a $(LIBTGT)d.a
	@echo "..... XTC LOCAL HEAP BUILD DONE / lib = $(LIBNAME) ....."

tests: dirs $(LIBTGT).a $(LIBTGT)d.a
	$(MAKE) -C tests BUILD=$(PKGBLD) LIBNAME=$(LIBNAME)
	@echo "..... XTC LOCAL HEAP TEST BUILD DONE / test = $(TSTNAME) ....."

examples: dirs $(LIBTGT).a $(LIBTGT)d.a
	$(MAKE) -C examples BUILD=$(PKGBLD) LIBNAME=$(LIBNAME)
	@echo "..... XTC LOCAL HEAP EXAMPLE BUILD DONE / example = $(XMPNAME) ....."

# Dependencies management

DEPS := $(SRCS:%=$(OBJD)/%.d)

$(OBJD)/%.d: $(SRCD)/%.c | $(OBJD)
	@$(CC) -MM -MP -MT $(OBJD)/$(basename $(<F)).o -MT $(OBJD)/$(basename $(<F))-g.o $(CFLAGS) $< > $@

# Target specificities

$(LIBTGT).a: CFLAGS += -O2 -DNDEBUG=1
$(LIBTGT)d.a: CFLAGS += -g -D__DEBUG

# Object files build rules

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

clean:
	@-$(RM) -r $(BUILD)
	@$(MAKE) -C tests clean
	@$(MAKE) -C examples clean

dirs: $(DIRS)

$(DIRS):
	@mkdir -p $@

.PHONY: all clean tests examples dirs

# Dependencies

-include $(DEPS)

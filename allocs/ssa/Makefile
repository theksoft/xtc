BUILD ?= build
LIBNAME ?= xtcssa

LIBD := $(BUILD)/lib
BIND := $(BUILD)/bin
OBJD := $(BUILD)/obj/ssa
DIRS := $(LIBD) $(BIND) $(OBJD)

LIBTGT := $(LIBD)/lib$(LIBNAME)

SRCS :=

CFLAGS += -Wall -Wextra -Werror -Wpedantic --pedantic-errors

AR := ar
ARFLAGS := rcs

all: dirs $(LIBTGT).a $(LIBTGT)d.a
	@echo "..... SSA BUILD DONE / lib = $(LIBNAME) ....."

tests: dirs $(LIBTGT).a $(LIBTGT)d.a
	@echo "..... SSA TEST BUILD DONE / test = $(TSTNAME) ....."

examples: dirs $(LIBTGT).a $(LIBTGT)d.a
	@echo "..... SSA EXAMPLE BUILD DONE / example = $(XMPNAME) ....."

$(LIBTGT).a: CFLAGS += -O2
$(LIBTGT)d.a: CFLAGS += -d -D__DEBUG

$(LIBTGT).a: $(SRCS:%=$(OBJD)/%.o)
	$(AR) $(ARFLAGS) $@ $^

$(LIBTGT)d.a: $(SRCS:%=$(OBJD)/%d.o)
	$(AR) $(ARFLAGS) $@ $^

clean: $(PKGS)
	@-rm -rf $(BUILD)

dirs: $(DIRS)

$(DIRS):
	@mkdir -p $@

.PHONY: all clean tests examples dirs $(DIRS)

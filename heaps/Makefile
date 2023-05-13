BUILD ?= ./build
LIBNAME ?= xtcheaps

PKGS := xstrhp xsyshp xgnrhp
PKGBLD := ../$(BUILD)
TARGET =

all: TARGET = all
tests: TARGET = tests
examples: TARGET = examples
clean: TARGET = clean
clean: PKGBLD = build

# Main target

all tests examples: $(PKGS)
	@echo "..... HEAPS BUILD DONE / lib = $(LIBNAME) ....."

# Package targets

$(PKGS):
	@echo
	$(MAKE) -C $@ BUILD=$(PKGBLD) LIBNAME=$(LIBNAME) $(TARGET)

# Other targets

clean: $(PKGS)
	@-$(RM) -r $(BUILD)

.PHONY: all clean tests examples $(PKGS)

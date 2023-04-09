BUILD ?= build
LIBNAME ?= xtcallocs

PKGS := ssa
PKGBLD := ../$(BUILD)
TARGET =

all: TARGET = all
tests: TARGET = tests
examples: TARGET = examples
clean: TARGET = clean
clean: PKGBLD = build

all tests examples: $(PKGS)
	@echo "----- ALLOCS BUILD DONE / lib = $(LIBNAME) / @ = $@ -----"

clean: $(PKGS)
	@-rm -rf $(BUILD)

$(PKGS):
	@echo
	$(MAKE) -C $@ BUILD=$(PKGBLD) LIBNAME=$(LIBNAME) $(TARGET)

.PHONY: all clean tests examples $(PKGS)

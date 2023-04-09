BUILD ?= build
LIBNAME ?= xtc

PRJS := allocs objs gofs
PRJBLD := ../$(BUILD)
TARGET =

all: TARGET = all
tests: TARGET = tests
examples: TARGET = examples
clean: TARGET = clean
clean: PRJBLD = build

all tests examples: $(PRJS)
	@echo
	@echo "===== XTC BUILD DONE / lib = $(LIBNAME) / @ = $@ ====="
	@echo

clean: $(PRJS)
	@-rm -rf $(BUILD)

$(PRJS):
	@echo
	$(MAKE) -C $@ BUILD=$(PRJBLD) LIBNAME=$(LIBNAME) $(TARGET)

.PHONY: all clean tests examples $(PRJS)

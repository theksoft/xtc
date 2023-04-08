BUILD ?= build

PROJS := allocs objs gofs
TARGET =

all: TARGET = all
tests: TARGET = tests
examples: TARGET = examples

all tests examples: $(PROJS)
	@echo
	@echo "===== XTC BUILD DONE / @ = $@ ====="
	@echo

$(PROJS):
	@echo
	$(MAKE) -C $@ BUILD=../$(BUILD) $(TARGET)

clean:
	@-rm -rf $(BUILD)

.PHONY: all clean tests examples $(PROJS)

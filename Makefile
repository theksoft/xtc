BUILD ?= build

all:
	$(MAKE) -C allocs BUILD=../$(BUILD)
	$(MAKE) -C objs BUILD=../$(BUILD)
	$(MAKE) -C gofs BUILD=../$(BUILD)
	@echo "===== XTC BUILD DONE ====="

tests:
	$(MAKE) -C allocs BUILD=../$(BUILD) tests
	$(MAKE) -C objs BUILD=../$(BUILD) tests
	$(MAKE) -C gofs BUILD=../$(BUILD) tests
	@echo "===== XTC TESTS BUILD DONE ====="

examples:
	$(MAKE) -C allocs BUILD=../$(BUILD) examples
	$(MAKE) -C objs BUILD=../$(BUILD) examples
	$(MAKE) -C gofs BUILD=../$(BUILD) examples
	@echo "===== XTC EXAMPLES BUILD DONE ====="

.PHONY: all tests examples

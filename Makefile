SCRIPTS := source/scripts
BUILD := build
STK := $(BUILD)/deps/stk-install/lib/libstk.a

.PHONY: cmake fixup clean setup 

all: cmake


cmake: $(LUAJIT) $(STK)
	@mkdir -p build && cd build && cmake .. && make

$(STK):
	@bash $(SCRIPTS)/build_stk.sh

fixup: cmake
	@bash $(SCRIPTS)/fix_bundle.sh


clean:
	@rm -rf build


setup:
	git submodule init
	git submodule update
	ln -s $(shell pwd) "$(HOME)/Documents/Max 8/Packages/$(shell basename `pwd`)"

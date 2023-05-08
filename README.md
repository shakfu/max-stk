# stk-max

A basic demo of using the [The Synthesis ToolKit (stk)](https://github.com/thestk/stk) library in Max/MSP externals.

Only tested on macOS.

## Installation

Just type the following:

```bash
git clone https://github.com/shakfu/stk-max.git
make setup
make
```

Note: `make setup` does the following:

```bash
git submodule init
git submodule update
ln -s $(shell pwd) "$(HOME)/Documents/Max 8/Packages/$(shell basename `pwd`)"
```

## Usage

Open `help/stk.sine~.maxhelp`.


## Todo

- [ ] implement more externals

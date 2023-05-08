#!/usr/bin/env bash

ROOT=`pwd`
export MACOSX_DEPLOYMENT_TARGET="10.11"

# $1 NAME
# $2 REPO
# $3 CMAKE-OPTS

function build_external_cmake_dep() {
	cd ${ROOT}
	echo `pwd`
	mkdir -p build/deps && \
	cd build/deps && \
	git clone --depth=1 ${2} ${1}-src && \
	mkdir ${1}-build && \
	cd ${1}-build && \
	cmake ${3} ../${1}-src && \
	make && \
	cmake --install . --prefix `pwd`/../${1}-install
}

function build_external_make_dep() {
	cd ${ROOT}
	echo `pwd`
	mkdir -p build/deps && \
	cd build/deps && \
	git clone ${2} ${1}-src && \
	cd ${1}-src && \
	make install PREFIX=`pwd`/../luajit-install
}

build_external_cmake_dep \
	stk \
	https://github.com/thestk/stk.git \
	-DENABLE_JACK=OFF


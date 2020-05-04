#!/bin/bash

if [ ! -d "../build" ]; then
	mkdir ../build
fi

pushd ../build >/dev/null

g++ -ggdb -o linux_main ../src/linux_main.cpp -lpthread 

popd >/dev/null





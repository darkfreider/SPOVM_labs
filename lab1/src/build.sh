#!/bin/bash

if [ ! -d "../build" ]; then
	mkdir ../build
fi

pushd ../build >/dev/null

g++ -ggdb -DGENERATE_PARENT_APP -o parent ../src/linux_main.cpp
g++ -ggdb -DGENERATE_CHILD_APP -o child ../src/linux_main.cpp

popd >/dev/null






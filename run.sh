#!/bin/bash

make clean;
make all;

chmod +x ./client;
chmod +x ./engine;


export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./engine/bin/Release;

./client/bin/Release/client


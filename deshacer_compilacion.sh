#!/bin/bash

cd global/deploy && make clean
cd ../../filesystem/deploy && make clean
cd ../../CPU/deploy && make clean
cd ../../kernel/deploy && make clean
cd ../../consola/deploy && make clean
cd ../../memoria/deploy && make clean

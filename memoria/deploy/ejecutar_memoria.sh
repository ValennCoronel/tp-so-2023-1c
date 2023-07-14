#!/bin/bash

LD_LIBRARY_PATH="/home/utnso/Desktop/tp-2023-1c-Grupo-SO-1/global/deploy" valgrind --leak-check=full --show-leak-kinds=all ./memoria

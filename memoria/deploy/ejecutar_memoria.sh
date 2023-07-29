#!/bin/bash

conValgrind=$1

if [ "$conValgrind" = "1" ]; then

        LD_LIBRARY_PATH="/home/utnso/Desktop/tp-2023-1c-Grupo-SO-1/global/deploy" valgrind --leak-check=full --show-leak-kinds=all --log-file=valgrind.log ./memoria

else
        LD_LIBRARY_PATH="/home/utnso/Desktop/tp-2023-1c-Grupo-SO-1/global/deploy" ./memoria
fi


#!/bin/bash

greenColour="\e[0;32m\033[1m"
endColour="\033[0m\e[0m"
redColour="\e[0;31m\033[1m"
blueColour="\e[0;34m\033[1m"
yellowColour="\e[0;33m\033[1m"
purpleColour="\e[0;35m\033[1m"
turquoiseColour="\e[0;36m\033[1m"
grayColour="\e[0;37m\033[1m"

if [ $# -ne 1 ]; then
	echo -e "${redColour}Parametro requerido${endColour}: ${yellowColour}<pathPseudocodigo>${endColour}"
	exit 1
fi

pathPseudocodigo="$1"

if [ ! -f "$pathPseudocodigo" ]; then
	echo -e "${redColour}No existe el pseudocodigo o esta mal escrito la ruta al pseudocodigo${endColour}"
	exit 1

fi

LD_LIBRARY_PATH="/home/utnso/Desktop/tp-2023-1c-Grupo-SO-1/global/deploy" ./consola ./consola.config "${pathPseudocodigo}"


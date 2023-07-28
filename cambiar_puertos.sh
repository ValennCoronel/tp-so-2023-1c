#!/bin/bash

greenColour="\e[0;32m\033[1m"
endColour="\033[0m\e[0m"
redColour="\e[0;31m\033[1m"
blueColour="\e[0;34m\033[1m"
yellowColour="\e[0;33m\033[1m"
purpleColour="\e[0;35m\033[1m"
turquoiseColour="\e[0;36m\033[1m"
grayColour="\e[0;37m\033[1m" 

function main() {
	if(($# != 4)); then
		echo -e "${redColour}Parametros requeridos${endColour}: ${yellowColour}<puertoMemoria> <puertoFilesystem> <puertoKernel> <puertoCPU>${endColour}"
		exit 1
	fi

	local -r puertoMemoria=$1
	local -r puertoFilesystem=$2
	local -r puertoKernel=$3
	local -r puertoCpu=$4

	perl -pi -e "s/(?<=PUERTO_MEMORIA=).*/${puertoMemoria}/g" CPU/deploy/cpu.config
	perl -pi -e "s/(?<=PUERTO_ESCUCHA=).*/${puertoCpu}/g" CPU/deploy/cpu.config

	perl -pi -e "s/(?<=PUERTO_MEMORIA=).*/${puertoMemoria}/g" filesystem/deploy/filesystem.config
	perl -pi -e "s/(?<=PUERTO_ESCUCHA=).*/${puertoFilesystem}/g" filesystem/deploy/filesystem.config

	perl -pi -e "s/(?<=PUERTO_ESCUCHA=).*/${puertoMemoria}/g" memoria/deploy/memoria.config

	perl -pi -e "s/(?<=PUERTO_MEMORIA=).*/${puertoMemoria}/g" kernel/deploy/kernel.config
	perl -pi -e "s/(?<=PUERTO_ESCUCHA=).*/${puertoKernel}/g" kernel/deploy/kernel.config
	perl -pi -e "s/(?<=PUERTO_FILESYSTEM=).*/${puertoFilesystem}/g" kernel/deploy/kernel.config
	perl -pi -e "s/(?<=PUERTO_CPU=).*/${puertoCpu}/g" kernel/deploy/kernel.config

	echo -e "${greenColour}[*]${endColour} ${grayColour}Puertos cambiados correctamente${endColour}"

}


main "$@"

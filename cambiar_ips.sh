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
	if(($# != 3)); then
		echo -e "${redColour}Parametros requeridos${endColour}: ${yellowColour}<memoriaIP> <cpuIP> <filesystemIP>${endColour}"
		exit 1
	fi

	local -r memoriaIP=$1
	local -r cpuIP=$2
	local -r filesystemIP=$3

	perl -pi -e "s/(?<=IP_MEMORIA=).*/${memoriaIP}/g" CPU/deploy/cpu.config
	perl -pi -e "s/(?<=IP_MEMORIA=).*/${memoriaIP}/g" filesystem/deploy/filesystem.config
	perl -pi -e "s/(?<=IP_MEMORIA=).*/${memoriaIP}/g" kernel/deploy/kernel.config

	perl -pi -e "s/(?<=IP_FILESYSTEM=).*/${filesystemIP}/g" kernel/deploy/kernel.config
	perl -pi -e "s/(?<=IP_CPU=).*/${cpuIP}/g" kernel/deploy/kernel.config

	echo -e "${greenColour}[*]${endColour} ${grayColour}IPs cambiadas correctamente${endColour}"
}


main "$@"

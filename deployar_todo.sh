#/bin/bash

#Colorcitos!!

greenColour="\e[0;32m\033[1m"
endColour="\033[0m\e[0m"
redColour="\e[0;31m\033[1m"
blueColour="\e[0;34m\033[1m"
yellowColour="\e[0;33m\033[1m"
purpleColour="\e[0;35m\033[1m"
turquoiseColour="\e[0;36m\033[1m"
grayColour="\e[0;37m\033[1m" 

# instalo las commons

echo -e "\n${greenColour}[*]${endColour}${grayColour}Instalando las commons ...${endColour}\n"


cd ..
#rm -rf "so-commons-library"
#git clone "https://github.com/sisoputnfrba/so-commons-library.git"
#cd so-commons-library
#make unistall install

cd tp-2023-1c-Grupo-SO-1

#echo -e "\n${greenColour}[*]${endColour}${grayColour}Bajando repo TP ...${endColour}\n"


#git clone "https://github.com/sisoputnfrba/tp-2023-1c-Grupo-SO-1.git"
#cd tp-2023-1c-Grupo-SO-1

#compilo librearia global

echo -e "\n${greenColour}[*]${endColour}${grayColour}Compilando Libreria global ...${endColour}\n\n"


cd global/deploy
make clean all

cd ../..

#compilo los modulos

echo -e "\n${greenColour}[*]${endColour}${grayColour}Compilando Memoria ...${endColour}\n\n"


cd memoria/deploy 
make clean all

target=$(find . -name memoria)
if [ "$target" != "./memoria" ]; then
	echo -e "\n${redColour}[*]${endColour}${grayColour}Error de compilacion. Saliendo...${endColour}\n\n"
	exit 0
fi


#ejecuto memoria
#LD_LIBRARY_PATH="/home/utnso/Desktop/tp-2023-1c-Grupo-SO-1/global/deploy" ./memoria &

echo -e "\n${greenColour}[*]${endColour}${grayColour}Compilando CPU ...${endColour}\n\n"


cd ../../CPU/deploy
make clean all
#LD_LIBRARY_PATH="/home/utnso/Desktop/tp-2023-1c-Grupo-SO-1/global/deploy" ./CPU 

target=$(find . -name CPU)
if [ "$target" != "./CPU" ]; then
	echo -e "\n${redColour}[*]${endColour}${grayColour}Error de compilacion. Saliendo...${endColour}\n\n"
	exit 0
fi

echo -e "\n${greenColour}[*]${endColour}${grayColour}Compilando Filesystem ...${endColour}\n\n"


cd ../../filesystem/deploy
make clean all
#LD_LIBRARY_PATH="/home/utnso/Desktop/tp-2023-1c-Grupo-SO-1/global/deploy" ./filesstem &

target=$(find . -name filesystem)
if [ "$target" != "./filesystem" ]; then
	echo -e "\n${redColour}[*]${endColour}${grayColour}Error de compilacion. Saliendo...${endColour}\n\n"
	exit 0
fi

echo -e "\n${greenColour}[*]${endColour}${grayColour}Compilando Kernel ...${endColour}\n\n"

cd ../../kernel/deploy
make clean all
#LD_LIBRARY_PATH="/home/utnso/Desktop/tp-2023-1c-Grupo-SO-1/global/deploy" ./kernel &

target=$(find . -name kernel)
if [ "$target" != "./kernel" ]; then
	echo -e "\n${redColour}[*]${endColour}${grayColour}Error de compilacion. Saliendo...${endColour}\n\n"
	exit 0
fi

echo -e "\n${greenColour}[*]${endColour}${grayColour}Compilando Consola ...${endColour}\n\n"


cd ../../consola/deploy
make clean all
#LD_LIBRARY_PATH="/home/utnso/Desktop/tp-2023-1c-Grupo-SO-1/global/deploy" ./consola ./consola.config ./codigo &

target=$(find . -name consola)
if [ "$target" != "./consola" ]; then
	echo -e "\n${redColour}[*]${endColour}${grayColour}Error de compilacion. Saliendo...${endColour}\n\n"
	exit 0
fi

echo -e "\n${yellowColour}[*]${endColour}${grayColour}Compialdo todo correctamente ...${endColour}\n\n"






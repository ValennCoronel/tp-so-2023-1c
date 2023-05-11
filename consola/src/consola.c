#include "consola.h"




int main(int argc, char** argv){


	//Iniciar logger y config

	t_log* logger = iniciar_logger();
	t_config* config = iniciar_config(argv[1]);


	//Declaracion variables para config
	char* ip_kernel;
	char* puerto_kernel;

	//Declaracion variables para test de conexion
	int conexion_kernel;


	//Chequeo que la cantidad de argumentos sea la correcta

	if(argc < 3){
		return EXIT_FAILURE;
	}



	//Testeo config
	if(config == NULL){
		log_error(logger, "No se pudo iniciar el archivo de configuración !!");

		terminar_programa(conexion_kernel, logger, config);
	}

	//Levantar datos de config a variables
	ip_kernel = config_get_string_value(config, "IP_KERNEL");
	puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");


	//Testeo de carga de variables
	if(!ip_kernel || !puerto_kernel ){
		log_error(logger, "Falta una de las siguientes propiedades en el archivo de configuración: 'PUERTO_KERNEL', 'IP_KERNEL");

		terminar_programa(conexion_kernel, logger, config);
	}

	//Realizo la conexion con memoria
	int result_conexion = conectar_modulo(conexion_kernel, ip_kernel, puerto_kernel);

	//Testeo el resultado de la conexion
	if(result_conexion == -1){
		log_error(logger, "Consola no se pudo conectar con el modulo Kernel !!");

		terminar_programa(conexion_kernel, logger, config);

	}

	log_info(logger, "La Consola se conecto con el modulo Kernel correctamente");



	//-------------------------------Envio de instrucciones---------------------------------------


	//Inicializar archivo de pseudocodigo
			FILE* archivo = fopen(argv[2],"r");


			//Comprobar si el archivo esta vacio
			if(archivo == NULL){
				perror("Error en la apertura del archivo");
				return 1;
			}


			//Declarar variables a utilizar

			char* cadena;
			cadena = malloc(30);

			//Leer archivo y extraer datos

			while(feof(archivo) == 0){
					fgets(cadena, 30, archivo);
					printf("cadena:  %s", cadena);
					instruccion *ptr_inst = (instruccion*) malloc(sizeof(instruccion)); //Creo la struct y reservo memoria

					char* token = strtok(cadena, " "); // obtiene el primer elemento en token
					ptr_inst->opcode = token; //Asigno la instruccion leida a la struct-> instruccion

					printf("%s\n", token); // imprime el primer elemento (solo por control)

					token = strtok(NULL, " "); // avanza al segundo elemento
					int i = 0; // Variable local utilizada para cargar el array de parametros

					    while (token != NULL) { //Ingresa si el parametro no es NULL

					    	ptr_inst->parametros[i] = token; //Carga el parametro en el array de la struct
					        token = strtok(NULL, " "); // obtiene el siguiente elemento
					        i++; //Avanza en el array

					    }

					    //Serializo y envio el paquete

					    paquete_instruccion(conexion_kernel, ptr_inst);
			}

			free(cadena);

			//Cerrar archivo
			fclose(archivo);
			printf("\n\n Se ha leido el archivo correctamente");



} //FIN DEL MAIN




//Funciones de inicio de Config y Logger
t_log* iniciar_logger(void){

	t_log* nuevo_logger = log_create("consola.log", "consola", true, LOG_LEVEL_INFO);

	return nuevo_logger;
}

t_config* iniciar_config(char* archivo){
	t_config* nueva_config = config_create(archivo);

	return nueva_config;
}



//Funcion para finalizar el programa
void terminar_programa(int conexion, t_log* logger, t_config* config){
	log_destroy(logger);
	config_destroy(config);
	close(conexion);
}


//Funcion para crear conexion entre modulos
int conectar_modulo(int conexion, char* ip, char* puerto){

	conexion = crear_conexion(ip, puerto);

	//enviar handshake
	enviar_mensaje("OK", conexion);

	int size;
	char* buffer = recibir_buffer(&size, conexion);

	if(strcmp(buffer, "ERROR") == 0 || strcmp(buffer, "") == 0){
		return -1;
	}

	return 0;

}

//Funcion para serializar, crear y enviar paquete de instruccion
void paquete_instruccion(int conexion, instruccion inst)
{
	// Declaro las variables a utilizar
	char* valor;
	t_paquete* paquete;
	t_buffer* buffer = malloc(sizeof(t_buffer)); //Creo buffer para serializar

	//Armado de buffer
	buffer->size = sizeof(uint32_t)*2 + sizeof(inst.opcode)+1 + sizeof(inst.parametros)+1;
	void* stream = malloc(buffer->size);
	int offset = 0; //Desplazamiento

	//Copiado de memoria

		//Opcode
	memcpy(stream + offset, &inst.opcode_lenght, sizeof(int));
	offset += sizeof(int);
	memcpy(stream + offset, inst.opcode, strlen(inst.opcode)+1);
	offset += strlen(inst.opcode)+1;

		//Parametros (tamaños)
	memcpy(stream + offset, &inst.parametro1_lenght, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &inst.parametro2_lenght, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &inst.parametro3_lenght, sizeof(uint32_t));
	offset += sizeof(uint32_t);

		//Parametros (valores)
	for(int i = 0; i< 3; i++){
	memcpy(stream + offset, inst.parametros[i], strlen(inst.parametros[i])+1);
	offset += strlen(inst.parametros[i])+1;
	}

	//Cargamos el buffer
	buffer->stream = stream;

	//Liberamos memoria dinamica
	free(inst.opcode);
	free(inst.parametros);

	//Asignamos el tamaño para el paquete y lo rellenamos
	paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = INSTRUCCION;
	paquete->buffer = buffer;


	//Armamos stream a enviar

	void* a_enviar = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
	offset = 0; //Reseteamos el offset

	memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));

	offset += sizeof(uint8_t);
	memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));

	offset += sizeof(uint32_t);
	memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

	// Enviamos el paquete
	send(conexion, a_enviar, buffer->size + sizeof(uint8_t) + sizeof(uint32_t), 0);

	// Liberamos memoria
	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}





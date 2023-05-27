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

	//Realizo la conexion con kernel
	int result_conexion = conectar_modulo(&conexion_kernel, ip_kernel, puerto_kernel);

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

	t_list* lista_instrucciones = list_create();

	//Leer archivo y extraer datos

	while(feof(archivo) == 0){
		cadena = malloc(30);
			fgets(cadena, 30, archivo);
			//printf("cadena:  %s", cadena);
			t_instruccion *ptr_inst = malloc(sizeof(t_instruccion)); //Creo la struct y reservo memoria

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
				ptr_inst->opcode_lenght = strlen(ptr_inst->opcode)+1;

				if(ptr_inst->parametros[0] != NULL){
					ptr_inst->parametro1_lenght = strlen(ptr_inst->parametros[0])+1;
				} else {
					ptr_inst->parametro1_lenght = 0;
				}
				if(ptr_inst->parametros[1] != NULL){
					ptr_inst->parametro2_lenght = strlen(ptr_inst->parametros[1])+1;
				} else {
					ptr_inst->parametro2_lenght = 0;
				}
				if(ptr_inst->parametros[2] != NULL){
					ptr_inst->parametro3_lenght = strlen(ptr_inst->parametros[2])+1;
				} else {
					ptr_inst->parametro3_lenght = 0;
				}

				list_add(lista_instrucciones, ptr_inst);

			}

			paquete_instruccion(conexion_kernel, lista_instrucciones);
			while(1);

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
int conectar_modulo(int *conexion, char* ip, char* puerto){

	*conexion = crear_conexion(ip, puerto);

	//enviar handshake
	enviar_mensaje("OK", *conexion);

	int size;
	char* buffer = recibir_buffer(&size, *conexion);

	if(strcmp(buffer, "ERROR") == 0 || strcmp(buffer, "") == 0){
		return -1;
	}

	return 0;

}

//Funcion para serializar, crear y enviar paquete de instruccion
void paquete_instruccion(int conexion, t_list* lista_instrucciones)
{
	// Declaro las variables a utilizar
	t_paquete* paquete;
	t_buffer* buffer = malloc(sizeof(t_buffer)); //Creo buffer para serializar

	int lista_length = list_size(lista_instrucciones);

	int buffer_size = sizeof(int); // entero para el tamanio de la lista de instrucciones
	void* stream = malloc(buffer_size);
	int offset = 0; //Desplazamiento

	memcpy(stream+offset, &(lista_length), sizeof(int));
	offset+= sizeof(int);

	//Copiado de memoria cada instruccion
	for(int i = 0; i< lista_length; i++){
		t_instruccion* inst = list_get(lista_instrucciones, i);

		buffer_size += sizeof(int)*1 // opcode_length
				+ inst->opcode_lenght //opcode
				+ sizeof(int)*3 // los 3 parametro_length
				+ inst->parametro1_lenght + inst->parametro2_lenght
				+ inst->parametro3_lenght;// el array de parametros
		// le reasigno mas tamaño
		stream = realloc(stream,buffer_size );

		//Opcode
		memcpy(stream + offset, &(inst->opcode_lenght), sizeof(int));
		offset += sizeof(int);
		memcpy(stream + offset, inst->opcode, inst->opcode_lenght);
		offset += inst->opcode_lenght;

			//Parametros (tamaños)
		memcpy(stream + offset, &(inst->parametro1_lenght), sizeof(int));
		offset += sizeof(int);
		memcpy(stream + offset, &(inst->parametro2_lenght), sizeof(int));
		offset += sizeof(int);
		memcpy(stream + offset, &(inst->parametro3_lenght), sizeof(int));
		offset += sizeof(int);

			//Parametros (valores)
		memcpy(stream + offset, inst->parametros[0], inst->parametro1_lenght);
		offset += inst->parametro1_lenght;
		memcpy(stream + offset, inst->parametros[1], inst->parametro2_lenght);
		offset +=inst->parametro2_lenght;
		memcpy(stream + offset, inst->parametros[2], inst->parametro3_lenght);
		offset += inst->parametro3_lenght;

		//Liberamos memoria dinamica
		//free(inst->opcode);
		//free(inst->parametros);
		//free(inst);
	}


	buffer->size = buffer_size;
	//Cargamos el buffer
	buffer->stream = stream;



	//Asignamos el tamaño para el paquete y lo rellenamos
	paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = INSTRUCCIONES;
	paquete->buffer = buffer;


	evniar_a_kernel(conexion,sizeof(op_code) + sizeof(int) + buffer->size , paquete);

	// Liberamos memoria
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}


void evniar_a_kernel(int conexion, int tamnio_paquete, t_paquete* paquete){

	void* a_enviar = malloc(tamnio_paquete);
	int offset = 0; //Reseteamos el offset

	memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));

	offset += sizeof(op_code);
	memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(int));

	offset += sizeof(int);
	memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

	// Enviamos el paquete
	send(conexion, a_enviar, tamnio_paquete, 0);

	// se queda en la espera de la finalizacion del proceso por parte del kernel
	int size;
	void* buffer_rcv = recibir_buffer(&size, conexion);

	free(a_enviar);
}


#include "cpu.h"

int main(void){

	//Declaracion variables para config
	char* retardo_instruccion;
	char* ip_memoria;
	char* puerto_memoria;
	char* puerto_escucha;
	char* tam_max_segmento;

	//Declaracion variables para test de conexion
	int conexion_memoria;

	//Iniciar logger y config

	logger = iniciar_logger();
	t_config* config = iniciar_config();


	//Testeo config
	if(config == NULL){
		log_error(logger, "No se pudo iniciar el archivo de configuración !!");

		terminar_programa(conexion_memoria, logger, config);
	}

	//Levantar datos de config a variables

	retardo_instruccion = config_get_int_value(config, "RETARDO_INSTRUCCION");
	ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
	puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
	tam_max_segmento = config_get_int_value(config, "TAM_MAX_SEGMENTO");

	//Testeo de carga de variables
	//if(!ip_memoria || !puerto_memoria || !puerto_escucha){
		//log_error(logger, "Falta una de las siguientes propiedades en el archivo de configuración: 'RETARDO_INSTRUCCION', 'IP_MEMORIA', 'PUERTO_MEMORIA', 'PUERTO_ESCUCHA', 'TAM_MAX_SEGMENTO'");

		//terminar_programa(conexion_memoria, logger, config);
	//}

	//Realizo la conexion con memoria
	int result_conexion_memoria = conectar_modulo(conexion_memoria, ip_memoria, puerto_memoria);

	//Testeo el resultado de la conexion
	if(result_conexion_memoria == -1){
		log_error(logger, "La CPU no se pudo conectar con el modulo Memoria !!");

		terminar_programa(conexion_memoria, logger, config);
	}

	log_info(logger, "La CPU se conecto con el modulo Memoria correctamente");

	//Escucho conexiones del Kernel
	int server_fd = iniciar_servidor(puerto_escucha);

	log_info(logger, "CPU listo para recibir peticiones del Kernel");

	// Maneja peciciones que recibe desde el kernel
	manejar_peticiones_kernel(logger, server_fd);

	/**
	 * Dentro de ejecutar estan las partes de
	 * fetch, buscar proxima instrucción
	 * recibir próxima instrucción
	 * decode, ver que codigo de operación
	 * ejecutar, respecto del código de operación
	 */
	ejecutar_instrucciones(server_fd, retardo_instruccion);

	terminar_programa(conexion_memoria, logger, config);

} //FIN DEL MAIN




//Funciones de inicio de Config y Logger
t_log* iniciar_logger(void){

	t_log* nuevo_logger = log_create("cpu.log", "CPU", true, LOG_LEVEL_INFO);

	return nuevo_logger;
}

t_config* iniciar_config(void){
	t_config* nueva_config = config_create("cpu.config");

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
	enviar_mensaje("OK", conexion, HANDSHAKE);

	int size;
	char* buffer = recibir_buffer(&size, conexion);

	if(strcmp(buffer, "ERROR") == 0 || strcmp(buffer, "") == 0){
		return -1;
	}

	return 0;
}

//Busco la prozima instruccion y actualizo el program counter y ejecuto las intrucciones
void ejecutar_instrucciones( int cliente_fd, char* retardo_instruccion )
{
	t_contexto_ejec* contexto = recibir_paquete_pcb (cliente_fd);
	int program_counter = contexto->program_counter;
	t_list *lista = contexto->lista_instrucciones;

	// Mientras haya instrucciones a ejecutar
	while( program_counter < lista->elements_count )
	{
		manejar_instruccion_kernel(cliente_fd, contexto, retardo_instruccion);
		// TODO ¿como actualizo el pcb que recibi del cliente?
		contexto->program_counter++; // Actualizar el programCounter para la próxima instrucción
	}
	// TODO ¿devolver algo porque ya no tiene mas instrucciones?
}

//Funcion para manejo de peticiones del kernel
void manejar_peticiones_kernel(t_log* logger, int server_fd){

	int cliente_fd = esperar_cliente(server_fd);


	while (1) {
			int cod_op = recibir_operacion(cliente_fd);

			switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(cliente_fd);
					break;
				case HANDSHAKE:
					recibir_handshake(cliente_fd);
					break;

				case CODIGO_OPERACION_RECIBIDO_POR_KERNEL:
					// TODO CODIGO_OPERACION_RECIBIDO_POR_KERNEL
					break;
				case -1:
					log_error(logger, "El cliente se desconecto. Terminando servidor");
					return;
				default:
					log_warning(logger,"Operacion desconocida. No quieras meter la pata");
					break;
			}
		}

	return ;
}

void manejar_instruccion_kernel(int cliente_fd, t_contexto_ejec** contexto, char* retardo_instruccion)
{
	t_instruccion* instruction = list_get((*contexto)->lista_instrucciones, (*contexto)->program_counter);

	if(strcmp(instruction->opcode,"YIELD")==0)
	{
		enviar_mensaje_a_kernel(DESALOJAR_PROCESO,cliente_fd, contexto);
		//poner contexto de ejecucion
	}
	if(strcmp(instruction->opcode,"EXIT")==0)
	{
		enviar_mensaje_a_kernel(FINALIZAR_PROCESO,cliente_fd, contexto);
		//poner contexto de ejecucion
		//TODO crear_paquete
	}

	// FIXME esto no es kernel
	if(strcmp(instruction->opcode,"SET")==0)
	{
		//A modo de simular el tiempo que transcurre en la CPU.
		sleep(retardo_instruccion);
		manejar_set(contexto);
	}
}

void manejar_instruccion_memoria(int cliente_fd, t_contexto_ejec** contexto)
{
	// TODO manejar_instruccion_memoria
}

void manejar_instruccion_filesystem(int cliente_fd, t_contexto_ejec** contexto)
{
	// TODO manejar_instruccion_filesystem
}

void enviar_mensaje_a_kernel(char* op_code, int cliente_fd, t_contexto_ejec* contexto)
{
	//elegir el mensaje que se enviara a kernel
	enviar_paquete()
}

void manejar_set(t_contexto_ejec** contexto,t_instruccion* instruccion)
{
	char* registro = instruccion -> parametros[0];
	char* valor = instruccion -> parametros[1];


	if(strcmp(registro,"AX")==0)
	{
		strcpy((*contexto)->registros_CPU->AX,string_subtstring_until(valor,4));

	}else if(strcmp(registro,"BX")==0)
	{
		strcpy((*contexto)->registros_CPU->BX,string_subtstring_until(valor,4));
	}else if(strcmp(registro,"CX")==0)
	{
		strcpy((*contexto)->registros_CPU->CX,string_subtstring_until(valor,4));
	}else if(strcmp(registro,"DX")==0)
	{
		strcpy((*contexto)->registros_CPU->DX,string_subtstring_until(valor,4));
	}else if(strcmp(registro,"EAX")==0)
	{
		strcpy((*contexto)->registros_CPU->EAX,string_subtstring_until(valor,8));
	}else if(strcmp(registro,"EBX")==0)
	{
		strcpy((*contexto)->registros_CPU->EBX,string_subtstring_until(valor,8));
	}else if(strcmp(registro,"ECX")==0)
	{
		strcpy((*contexto)->registros_CPU->ECX,string_subtstring_until(valor,8));
	}else if(strcmp(registro,"EDX")==0)
	{
		strcpy((*contexto)->registros_CPU->EDX,string_subtstring_until(valor,8));
	}else if(strcmp(registro,"RAX")==0)
	{
		strcpy((*contexto)->registros_CPU->RAX,string_subtstring_until(valor,16));
	}else if(strcmp(registro,"RBX")==0)
	{
		strcpy((*contexto)->registros_CPU->RBX,string_subtstring_until(valor,16));
	}else if(strcmp(registro,"RCX")==0)
	{
		strcpy((*contexto)->registros_CPU->RCX,string_subtstring_until(valor,16));
	}else if(strcmp(registro,"RDX")==0)
	{
		strcpy((*contexto)->registros_CPU->RDX,string_subtstring_until(valor,16));
	}
}

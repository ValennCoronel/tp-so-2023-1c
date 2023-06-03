#include "cpu.h"

int main(void){

	//Declaracion variables para config
	int RETARDO_INSTRUCCION;
	int TAM_MAX_SEGMENTO;
	char* ip_memoria;
	char* puerto_memoria;
	char* puerto_escucha;
	int tam_max_segmento;

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

	RETARDO_INSTRUCCION = config_get_int_value(config, "RETARDO_INSTRUCCION");
	TAM_MAX_SEGMENTO = config_get_int_value(config, "TAM_MAX_SEGMENTO");

	ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
	puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");


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

	// Escucho conexiones del Kernel
	int server_fd = iniciar_servidor(puerto_escucha);

	log_info(logger, "CPU listo para recibir peticiones del Kernel");

	//inicializar_colas_y_semaforos();
	//pthread_t thread_consolas, thread_planificador_largo_plazo, thread_planificador_corto_plazo;

	// Maneja peciciones que recibe desde el kernel
	//pthread_create(&thread_consolas, NULL, manejar_peticiones_kernel, args_consolas);
	/**
	 * typedef struct {
	int server_fd;
	int retardo_instruccion;
}manejar_peticiones_kernel_args;
	 *
	 */

	escuchar_peticiones_kernel(logger, server_fd, RETARDO_INSTRUCCION,TAM_MAX_SEGMENTO);

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
int conectar_modulo(int* conexion, char* ip, char* puerto){

	*conexion = crear_conexion(ip, puerto);

	//enviar handshake
	enviar_mensaje("OK", *conexion, HANDSHAKE);

	int size;
	char* buffer = recibir_buffer(&size, *conexion);

	if(strcmp(buffer, "ERROR") == 0 || strcmp(buffer, "") == 0){
		return -1;
	}

	return 0;
}

//Funcion para manejo de peticiones del kernel:
void escuchar_peticiones_kernel(t_log* logger, int server_fd, int RETARDO_INSTRUCCION, int TAM_MAX_SEGMENTO){

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
				case PETICION_CPU:
					manejar_peticion_al_cpu(cliente_fd, RETARDO_INSTRUCCION, TAM_MAX_SEGMENTO);
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
/**
 * Fetch, Decode
 */
void manejar_peticion_al_cpu(int cliente_fd, int RETARDO_INSTRUCCION, int TAM_MAX_SEGMENTO)
{
	t_contexto_ejec* contexto = recibir_paquete_pcb (cliente_fd);
	int program_counter = contexto->program_counter;
	t_list *lista = contexto->lista_instrucciones;

	t_instruccion* instruction = list_get(lista, (contexto)->program_counter-1);

	if(strcmp(instruction->opcode,"SET")==0)
	{
		//A modo de simular el tiempo que transcurre en la CPU.
		sleep(RETARDO_INSTRUCCION);
		manejar_instruccion_set(&contexto, instruction);
	}

	if(strcmp(instruction->opcode,"MOV_IN")==0)
	{
		manejar_instruccion_mov_in(cliente_fd, &contexto, instruction);
	}

	if(strcmp(instruction->opcode,"MOV_OUT")==0)
	{
		manejar_instruccion_mov_out(contexto, instruction,cliente_fd, TAM_MAX_SEGMENTO);
	}

	if(strcmp(instruction->opcode,"YIELD")==0)
	{
		enviar_mensaje_a_kernel(DESALOJAR_PROCESO,cliente_fd, &contexto);
		//poner contexto de ejecucion
	}
	if(strcmp(instruction->opcode,"EXIT")==0)
	{
		enviar_mensaje_a_kernel(FINALIZAR_PROCESO,cliente_fd, &contexto);
	}
	if(strcmp(instruction->opcode,"WAIT")==0)
	{
		enviar_mensaje_a_kernel(APROPIAR_RECURSOS,cliente_fd, &contexto);
	}
}

void enviar_mensaje_a_kernel(op_code code,int cliente_fd,t_contexto_ejec** contexto){

	t_paquete* paquete = crear_paquete(code);

	agregar_a_paquete_sin_agregar_tamanio(paquete, (*contexto)->tamanio_lista, sizeof(int));

	for(int i =0; i< (*contexto)->tamanio_lista; i++){
		t_instruccion* instruccion = list_get( (*contexto)->lista_instrucciones, i);

		agregar_a_paquete(paquete, instruccion->opcode, sizeof(char)*instruccion->opcode_lenght);

		agregar_a_paquete(paquete, instruccion->parametros[0], instruccion->parametro1_lenght);
		agregar_a_paquete(paquete, instruccion->parametros[1], instruccion->parametro2_lenght);
		agregar_a_paquete(paquete, instruccion->parametros[2], instruccion->parametro3_lenght);

	}

	agregar_a_paquete_sin_agregar_tamanio(paquete,  (*contexto)->program_counter, sizeof(int));

	agregar_a_paquete(paquete,  (*contexto)->registros_CPU->AX, sizeof(char)*4);
	agregar_a_paquete(paquete,  (*contexto)->registros_CPU->BX, sizeof(char)*4);
	agregar_a_paquete(paquete,  (*contexto)->registros_CPU->CX, sizeof(char)*4);
	agregar_a_paquete(paquete,  (*contexto)->registros_CPU->DX, sizeof(char)*4);

	agregar_a_paquete(paquete,  (*contexto)->registros_CPU->EAX, sizeof(char)*8);
	agregar_a_paquete(paquete,  (*contexto)->registros_CPU->EBX, sizeof(char)*8);
	agregar_a_paquete(paquete,  (*contexto)->registros_CPU->ECX, sizeof(char)*8);
	agregar_a_paquete(paquete,  (*contexto)->registros_CPU->EDX, sizeof(char)*8);

	agregar_a_paquete(paquete,  (*contexto)->registros_CPU->RAX, sizeof(char)*16);
	agregar_a_paquete(paquete,  (*contexto)->registros_CPU->RBX, sizeof(char)*16);
	agregar_a_paquete(paquete,  (*contexto)->registros_CPU->RCX, sizeof(char)*16);
	agregar_a_paquete(paquete,  (*contexto)->registros_CPU->RDX, sizeof(char)*16);

	enviar_paquete(paquete, cliente_fd);
	eliminar_paquete(paquete);
}

void manejar_instruccion_set(t_contexto_ejec** contexto,t_instruccion* instruccion)
{
	char* registro = strdup(instruccion->parametros[0]);
	char* valor = strdup(instruccion->parametros[1]);

	if(strcmp(registro,"AX")==0)
	{
		strcpy((*contexto)->registros_CPU->AX, string_substring_until(valor,4));

	}else if(strcmp(registro,"BX")==0)
	{
		strcpy((*contexto)->registros_CPU->BX, string_substring_until(valor,4));
	}else if(strcmp(registro,"CX")==0)
	{
		strcpy((*contexto)->registros_CPU->CX, string_substring_until(valor,4));
	}else if(strcmp(registro,"DX")==0)
	{
		strcpy((*contexto)->registros_CPU->DX, string_substring_until(valor,4));
	}else if(strcmp(registro,"EAX")==0)
	{
		strcpy((*contexto)->registros_CPU->EAX, string_substring_until(valor,8));
	}else if(strcmp(registro,"EBX")==0)
	{
		strcpy((*contexto)->registros_CPU->EBX,string_substring_until(valor,8));
	}else if(strcmp(registro,"ECX")==0)
	{
		strcpy((*contexto)->registros_CPU->ECX,string_substring_until(valor,8));
	}else if(strcmp(registro,"EDX")==0)
	{
		strcpy((*contexto)->registros_CPU->EDX,string_substring_until(valor,8));
	}else if(strcmp(registro,"RAX")==0)
	{
		strcpy((*contexto)->registros_CPU->RAX,string_substring_until(valor,16));
	}else if(strcmp(registro,"RBX")==0)
	{
		strcpy((*contexto)->registros_CPU->RBX,string_substring_until(valor,16));
	}else if(strcmp(registro,"RCX")==0)
	{
		strcpy((*contexto)->registros_CPU->RCX,string_substring_until(valor,16));
	}else if(strcmp(registro,"RDX")==0)
	{
		strcpy((*contexto)->registros_CPU->RDX,string_substring_until(valor,16));
	}
}

void traducir_direccion_memoria(int direccion_logica, int TAM_MAX_SEGMENTO)
{
	int num_segmento = floor(direccion_logica / TAM_MAX_SEGMENTO);
	int desplazamiento_segmento = direccion_logica % TAM_MAX_SEGMENTO;
}
/**
 * (Registro, Dirección Lógica)
 * Lee el valor de memoria correspondiente a la Dirección Lógica y lo almacena en el Registro.
 *
 */
void manejar_instruccion_mov_in(int cliente_fd, t_contexto_ejec** contexto,t_instruccion* instruccion)
{
	//Dame el contenido de la memoria, y lo pongo en el registro
	//t_paquete* paquete = crear_paquete(READ_MEMORY);
	//dir_logica = traducir_direccion_memoria(int direccion_logica, int TAM_MAX_SEGMENTO)
	//Armar el paquete con la direccion de la memoria
	//enviar_paquete(paquete, cliente_fd);

	//FIXME No estoy seguro de esto, deveria devolver el contenido de la memoria
	//void buffer = recibir_buffer( size, socket_cliente)(*sizeof(int), int socket_cliente );
	//setear registro con lo devuelto
}
/**
 * (Dirección Lógica, Registro)
 * Lee el valor del Registro y lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica.
 *
 */
void manejar_instruccion_mov_out(t_contexto_ejec* contexto, t_instruccion* instruction, int cliente_fd, int TAM_MAX_SEGMENTO)
{
	//t_paquete* paquete = crear_paquete(WRITE_MEMORY);
	//enviar_paquete(paquete, cliente_fd);
	//void buffer = recibir_buffer(*sizeof(int), int socket_cliente); // Debe de decir ok
}



